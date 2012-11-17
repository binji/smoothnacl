// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/copybuffercr.h"
#include "gen/shader_source.h"
#include "gpu/texture.h"
#include "gpu/wrap_gl.h"

namespace gpu {

CopyBufferCR::CopyBufferCR(const pp::Size& size)
    : size_(size) {
  int w = size_.width();
  int h = size_.height();
  shader_.Init(shader_source_copybuffercr_frag, shader_source_2tex_vert);
  vb_.SetSize(w, h);
  vb_.SetTex(0, 0, 0, 1-1.f/(w/2+1), 1);
  vb_.SetTex(1, 0, 0, w, 1);
  vb_.LoadData();
}

CopyBufferCR::~CopyBufferCR() {
}

void CopyBufferCR::Apply(const Texture& in, Texture& out) {
  int w = size_.width();
  int h = size_.height();
  shader_.Use();
  shader_.UniformMatrixOrtho("u_mat", 0, w, 0, h, -1, 1);
  shader_.UniformTexture("u_tex0", 0, in);
  shader_.UniformTexture("u_tex1", 1, in);
  vb_.SetAttribs(shader_.GetAttribLocation("a_position"),
                 shader_.GetAttribLocation("a_texcoord0"),
                 shader_.GetAttribLocation("a_texcoord1"));
  out.BindFramebuffer();
  glViewport(0, 0, w, h);
  vb_.Draw();
  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}  // namespace gpu
