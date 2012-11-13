// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/kernel_mul.h"
#include "gen/shader_source.h"
#include "gpu/texture.h"
#include "gpu/wrap_gl.h"

namespace gpu {

KernelMul::KernelMul(const pp::Size& size)
    : size_(size) {
  shader_.Init(shader_source_kernelmul_frag, shader_source_kernelmul_vert);
  vb_.SetSize(size_.width()/2 + 1, size_.height());
  vb_.SetTex(0, 0, 0, 1, 1);
  vb_.SetTex(1, 0, 0, 1, 1);
  vb_.LoadData();
}

KernelMul::~KernelMul() {
}

void KernelMul::Apply(const Texture& in0, const Texture& in1, Texture& out,
                      double scale) {
  int w = size_.width();
  int h = size_.height();
  shader_.Use();
  shader_.UniformMatrixOrtho("u_mat", 0, w/2 + 1, 0, h, -1, 1);
  shader_.Uniform1f("sc", scale);
  shader_.UniformTexture("tex0", 0, in0);
  shader_.UniformTexture("tex1", 1, in1);
  vb_.SetAttribs(shader_.GetAttribLocation("a_position"),
                 shader_.GetAttribLocation("a_texcoord0"),
                 shader_.GetAttribLocation("a_texcoord1"));
  out.BindFramebuffer();
  glViewport(0, 0, w/2 + 1, h);
  vb_.Draw();
  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}  // namespace gpu
