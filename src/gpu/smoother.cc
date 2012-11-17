// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/smoother.h"
#include "gen/shader_source.h"
#include "gpu/texture.h"
#include "gpu/wrap_gl.h"

namespace gpu {

Smoother::Smoother(const pp::Size& size, const SmootherConfig& config)
    : size_(size),
      config_(config) {
  shader_.Init(shader_source_smoother_frag, shader_source_3tex_vert);
  vb_.SetSize(1, 1);
  vb_.SetTex(0, 0, 0, 1, 1);
  vb_.SetTex(1, 0, 0, 1, 1);
  vb_.SetTex(2, 0, 0, 1, 1);
  vb_.LoadData();
}

Smoother::~Smoother() {
}

void Smoother::SetConfig(const SmootherConfig& config) {
  config_ = config;
}

void Smoother::Apply(const Texture& in0, const Texture& in1, Texture& out) {
  shader_.Use();
  shader_.UniformMatrixOrtho("u_mat", 0, 1, 0, 1, -1, 1);
  shader_.Uniform1i("u_mode", config_.timestep.type);
  shader_.Uniform1f("u_dt", config_.timestep.dt);
  shader_.Uniform1f("u_b1", config_.b1);
  shader_.Uniform1f("u_b2", config_.b2);
  shader_.Uniform1f("u_d1", config_.d1);
  shader_.Uniform1f("u_d2", config_.d2);
  shader_.Uniform1i("u_sigmode", config_.mode);
  shader_.Uniform1i("u_sigtype", config_.sigmoid);
  shader_.Uniform1i("u_mixtype", config_.mix);
  shader_.Uniform1f("u_sn", config_.sn);
  shader_.Uniform1f("u_sm", config_.sm);
  shader_.UniformTexture("u_tex0", 0, in0);
  shader_.UniformTexture("u_tex1", 1, in1);
  shader_.UniformTexture("u_tex2", 2, out);
  vb_.SetAttribs(shader_.GetAttribLocation("a_position"),
                 shader_.GetAttribLocation("a_texcoord0"),
                 shader_.GetAttribLocation("a_texcoord1"),
                 shader_.GetAttribLocation("a_texcoord2"));
  out.BindFramebuffer();
  glViewport(0, 0, size_.width(), size_.height());
  vb_.Draw();
  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}  // namespace gpu
