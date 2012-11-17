// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/draw_circle.h"
#include "gen/shader_source.h"
#include "gpu/texture.h"
#include "gpu/wrap_gl.h"

namespace gpu {

DrawCircle::DrawCircle(const pp::Size& size)
    : size_(size) {
  shader_.Init(shader_source_draw_circle_frag, shader_source_draw_circle_vert);
  vb_.SetSize(1, 1);
  vb_.LoadData();
}

DrawCircle::~DrawCircle() {
}

void DrawCircle::Apply(Texture& inout, float x, float y, float radius) {
  int w = size_.width();
  int h = size_.height();
  shader_.Use();
  shader_.UniformMatrixOrtho("u_mat", 0, w, 0, h, -1, 1);
  shader_.Uniform2f("u_window_size", w, h);
  shader_.Uniform2f("u_pos", x, y);
  shader_.Uniform1f("u_radius", radius);
  shader_.UniformTexture("u_tex0", 0, inout);
  vb_.SetAttribs(shader_.GetAttribLocation("a_position"));
  inout.BindFramebuffer();
  glViewport(0, 0, w, h);
  vb_.Draw();
  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}  // namespace gpu
