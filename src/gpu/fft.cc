// Copyright 2013 Ben Smith. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gpu/fft.h"
#include <assert.h>
#include <math.h>
#include "gen/shader_source.h"
#include "gpu/texture.h"
#include "gpu/wrap_gl.h"

namespace gpu {

FFT::FFT(const pp::Size& size)
    : size_(size),
      complex_to_real_(size),
      real_to_complex_(size),
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

  real_to_complex_.Apply(in, *src);
  for (int index = 1; index <= fft_stage_.log2w(); ++index) {
    fft_stage_.ApplyX(index, FFT_SIGN_NEGATIVE, *src, *dst);
    std::swap(src, dst);
  }
  for (int index = 1; index <= fft_stage_.log2h(); ++index) {
    if (index == fft_stage_.log2h())
      dst = &out;
    fft_stage_.ApplyY(index, FFT_SIGN_NEGATIVE, *src, *dst);
    std::swap(src, dst);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FFT::ApplyCR(const Texture& in, Texture& out) {
  Texture* src = &tex_;
  Texture* dst = &tex2_;

  for (int index = 1; index <= fft_stage_.log2h(); ++index) {
    fft_stage_.ApplyY(index, FFT_SIGN_POSITIVE, (index == 1 ? in : *src), *dst);
    std::swap(src, dst);
  }
  for (int index = 0; index <= fft_stage_.log2w() - 1; ++index) {
    fft_stage_.ApplyX(index, FFT_SIGN_POSITIVE, *src, *dst);
    std::swap(src, dst);
  }
  complex_to_real_.Apply(*src, out);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}  // namespace gpu
