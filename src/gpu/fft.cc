// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/fft.h"
#include <assert.h>
#include <math.h>
#include "gen/shader_source.h"
#include "gpu/texture.h"
#include "gpu/wrap_gl.h"

namespace gpu {

FFT::FFT(const pp::Size& size)
    : size_(size),
      copybuffercr_(size),
      copybufferrc_(size),
      fft_stage_(size),
      tex_(size.width()/2 + 1, size.height(), FORMAT_COMPLEX,
           TEXTURE_FRAMEBUFFER),
      tex2_(size.width()/2 + 1, size.height(), FORMAT_COMPLEX,
            TEXTURE_FRAMEBUFFER) {
}

FFT::~FFT() {
}

void FFT::ApplyRC(const Texture& in, Texture& out) {
  Texture* src = &tex_;
  Texture* dst = &tex2_;

  copybufferrc_.Apply(in, *src);
  for (int index = 1; index <= fft_stage_.log2w(); ++index) {
    fft_stage_.ApplyX(index, FFT_SIGN_POSITIVE, *src, *dst);
    std::swap(src, dst);
  }
  for (int index = 1; index <= fft_stage_.log2h(); ++index) {
    if (index == fft_stage_.log2h())
      dst = &out;
    fft_stage_.ApplyY(index, FFT_SIGN_POSITIVE, *src, *dst);
    std::swap(src, dst);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FFT::ApplyCR(const Texture& in, Texture& out) {
  Texture* src = &tex_;
  Texture* dst = &tex2_;

  for (int index = 1; index <= fft_stage_.log2h(); ++index) {
    fft_stage_.ApplyY(index, FFT_SIGN_NEGATIVE, index == 1 ? in : *src, *dst);
    std::swap(src, dst);
  }
  for (int index = 1; index <= fft_stage_.log2w(); ++index) {
    fft_stage_.ApplyX(index, FFT_SIGN_NEGATIVE, *src, *dst);
    std::swap(src, dst);
  }
  copybuffercr_.Apply(*src, out);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}  // namespace gpu
