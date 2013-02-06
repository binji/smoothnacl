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

void DrawCircle::Apply(Texture& inout, const Circles& circles) {
  int w = size_.width();
  int h = size_.height();
  shader_.Use();
  shader_.UniformMatrixOrtho("u_mat", 0, w, 0, h, -1, 1);
  shader_.Uniform2f("u_window_size", w, h);
  shader_.UniformTexture("u_tex0", 0, inout);
  vb_.SetAttribs(shader_.GetAttribLocation("a_position"));
  inout.BindFramebuffer();
  glViewport(0, 0, w, h);
  for (int i = 0; i < circles.size(); ++i) {
    const Circle& circle = circles[i];
    shader_.Uniform2f("u_pos", circle.x, circle.y);
    shader_.Uniform1f("u_radius", circle.radius);
    vb_.Draw();
  }
  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}  // namespace gpu
