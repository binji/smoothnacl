// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/fft_stage.h"
#include <assert.h>
#include <math.h>
#include "gen/shader_source.h"
#include "gpu/texture.h"
#include "gpu/wrap_gl.h"

namespace gpu {
namespace {

const float kPi = 3.14159265358979323846;

uint32_t IsPowerOf2(uint32_t x) {
  return x && (x & (x - 1)) == 0;
}

uint32_t Log2(uint32_t x) {
  assert(IsPowerOf2(x));
  uint32_t result = 0;
  --x;
  while (x) {
    result++;
    x >>= 1;
  }
  return result;
}

int BitReverse(int x, int b) {
  int result = 0;
  for (int t = 0; t < b; t++)
    result = (result << 1) | ((x >> t) & 1);
  return result;
}

}  // namespace

FFTStage::FFTStage(const pp::Size& size)
    : size_(size),
      log2w_(Log2(size.width())),
      log2h_(Log2(size.height())) {
  int w = size_.width();
  int h = size_.height();
  shader_.Init(shader_source_fft_frag, shader_source_fft_vert);
  vb_.SetSize(w/2 + 1, h);
  vb_.SetTex(0, 0, 0, 1, 1);
  vb_.SetTex(1, 0, 0, 1, 1);
  vb_.LoadData();
  vb2_.SetSize(w/2, h);
  vb2_.SetTex(0, 0, 0, 1 - 1.f / (w/2 + 1), 1);
  vb2_.SetTex(1, 0, 0, 1 - 1.f / (w/2 + 1), 1);
  vb2_.LoadData();
  MakeAllPlanX();
  MakeAllPlanY();
}

FFTStage::~FFTStage() {
}

void FFTStage::ApplyX(int index, FFTSign sign, const Texture& in,
                      Texture& out) {
  int w = size_.width();
  int h = size_.height();
  int tang;
  float tangsc;
  VertexBuffer* fft_vb = NULL;

  if (sign == FFT_SIGN_POSITIVE && index == 0) {
    tang = 1;
    tangsc = 0.5 * sqrt(2.0);
    fft_vb = &vb2_;
  } else if (sign == FFT_SIGN_NEGATIVE && index == log2w_) {
    tang = 1;
    tangsc = 0.5 / sqrt(2.0);
    fft_vb = &vb_;
  } else {
    tang = 0;
    tangsc = 0;
    fft_vb = &vb2_;
  }

  shader_.Use();
  shader_.UniformMatrixOrtho("u_mat", 0, w/2 + 1, 0, h, -1, 1);
  shader_.Uniform1i("dim", 1);
  shader_.Uniform1i("tang", tang);
  shader_.Uniform1f("tangsc", tangsc);
  shader_.UniformTexture("tex0", 0, in);
  shader_.UniformTexture("tex1", 1, *planx_[index][sign]);
  fft_vb->SetAttribs(shader_.GetAttribLocation("a_position"),
                     shader_.GetAttribLocation("a_texcoord0"),
                     shader_.GetAttribLocation("a_texcoord1"));
  out.BindFramebuffer();
  glViewport(0, 0, w/2 + 1, h);
  fft_vb->Draw();
  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FFTStage::ApplyY(int index, FFTSign sign, const Texture& in,
                      Texture& out) {
  int w = size_.width();
  int h = size_.height();
  shader_.Use();
  shader_.UniformMatrixOrtho("u_mat", 0, w/2 + 1, 0, h, -1, 1);
  shader_.Uniform1i("dim", 2);
  shader_.Uniform1i("tang", 0);
  shader_.Uniform1f("tangsc", 0);
  shader_.UniformTexture("tex0", 0, in);
  shader_.UniformTexture("tex1", 1, *plany_[index][sign]);
  vb_.SetAttribs(shader_.GetAttribLocation("a_position"),
                 shader_.GetAttribLocation("a_texcoord0"),
                 shader_.GetAttribLocation("a_texcoord1"));
  out.BindFramebuffer();
  glViewport(0, 0, w/2 + 1, h);
  vb_.Draw();
  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FFTStage::MakeAllPlanX() {
  int texture_width = size_.width()/2 + 1;
  float4* buffer = new float4[texture_width];
  planx_ = new Texture2[log2w_ + 1];

  for (int s = 0; s <= 1; s++) {
    for (int index = 0; index <= log2w_; index++) {
      MakePlanX(s * 2 - 1, index, buffer);
      Texture* texture = new Texture(texture_width, 1, FORMAT_4FLOAT,
                                     TEXTURE_NOFRAMEBUFFER);
      texture->Load(buffer, texture_width, 1);
      planx_[index][s] = texture;
    }
  }

  delete [] buffer;
}

void FFTStage::MakePlanX(float sign, int index, float4* buffer) {
  const size_t w = size_.width();
  const float fw = size_.width();

  for (int x = 0; x < w/2 + 1; x++) {
    if (sign == 1 && index == 0) {
      buffer[x][0] = (       x + 0.5f) / (fw/2 + 1);
      buffer[x][1] = (fw/2 - x + 0.5f) / (fw/2 + 1);
      double angle = 2 * sign * kPi * (x / fw + 0.25);
      buffer[x][2] = cos(angle);
      buffer[x][3] = sin(angle);
    }
    else if (sign == -1 && index == log2w_) {
      if (x == 0 || x == w/2) {
        buffer[x][0] = 0;
        buffer[x][1] = 0;
      } else {
        buffer[x][0] = (       x) / (fw/2);
        buffer[x][1] = (fw/2 - x) / (fw/2);
      }
      double angle = 2 * sign * kPi * (x / fw + 0.25);
      buffer[x][2] = cos(angle);
      buffer[x][3] = sin(angle);
    }
    else if (x < w/2) {
      int l = 1 << index;
      int j = x % l;
      float angle = 2 * sign * kPi * j / l;

      if (j < l/2) {
        if (index == 1) {
          buffer[x][0] = BitReverse(x      , log2w_ - 1) / (fw/2);
          buffer[x][1] = BitReverse(x + l/2, log2w_ - 1) / (fw/2);
        } else {
          buffer[x][0] = (x      ) / (fw/2);
          buffer[x][1] = (x + l/2) / (fw/2);
        }
      } else {
        if (index == 1) {
          buffer[x][0] = BitReverse(x - l/2, log2w_ - 1) / (fw/2);
          buffer[x][1] = BitReverse(x      , log2w_ - 1) / (fw/2);
        } else {
          buffer[x][0] = (x - l/2) / (fw/2);
          buffer[x][1] = (x      ) / (fw/2);
        }
      }
      buffer[x][2] = cos(angle);
      buffer[x][3] = sin(angle);
    } else {
      buffer[x][0] = 0;
      buffer[x][1] = 0;
      buffer[x][2] = 0;
      buffer[x][3] = 0;
    }

  }
}

void FFTStage::MakeAllPlanY() {
  int texture_width = size_.height();
  float4* buffer = new float4[texture_width];
  plany_ = new Texture2[log2h_ + 1];

  for (int s = 0; s <= 1; s++) {
    for (int index = 1; index <= log2h_; index++) {
      MakePlanY(s * 2 - 1, index, buffer);
      Texture* texture = new Texture(texture_width, 1, FORMAT_4FLOAT,
                                     TEXTURE_NOFRAMEBUFFER);
      texture->Load(buffer, texture_width, 1);
      plany_[index][s] = texture;
    }
  }

  delete [] buffer;
}

void FFTStage::MakePlanY(float sign, int index, float4* buffer) {
  const float fh = size_.height();

  for (int x = 0; x < size_.height(); x++) {
    int l = 1 << index;
    int j = x % l;
    if (j < l/2) {
      if (index == 1) {
        buffer[x][0] = BitReverse(x      , log2h_) / fh;
        buffer[x][1] = BitReverse(x + l/2, log2h_) / fh;
      } else {
        buffer[x][0] = (x      ) / fh;
        buffer[x][1] = (x + l/2) / fh;
      }
    } else {
      if (index == 1) {
        buffer[x][0] = BitReverse(x - l/2, log2h_) / fh;
        buffer[x][1] = BitReverse(x      , log2h_) / fh;
      } else {
        buffer[x][0] = (x - l/2) / fh;
        buffer[x][1] = (x      ) / fh;
      }
    }
    float angle = 2 * sign * kPi * j / l;
    buffer[x][2] = cos(angle);
    buffer[x][3] = sin(angle);
  }
}

}  // namespace gpu
