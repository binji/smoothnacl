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

#include "gpu/real_to_complex.h"
#include "gen/shader_source.h"
#include "gpu/texture.h"
#include "gpu/wrap_gl.h"

namespace gpu {

RealToComplex::RealToComplex(const pp::Size& size)
    : size_(size) {
  int w = size_.width();
  int h = size_.height();
  shader_.Init(shader_source_real_complex_frag, shader_source_2tex_vert);
  vb_.SetSize(w/2, h);
  vb_.SetTex(0, -1.0f/w, 0, 1 - 1.0f/w, 1);
  vb_.SetTex(1, 0, 0, 1, 1);
  vb_.LoadData();
}

RealToComplex::~RealToComplex() {
}

void RealToComplex::Apply(const Texture& in, Texture& out) {
  int w = size_.width();
  int h = size_.height();
  shader_.Use();
  shader_.UniformMatrixOrtho("u_mat", 0, w/2+1, 0, h, -1, 1);
  shader_.UniformTexture("u_tex0", 0, in);
  shader_.UniformTexture("u_tex1", 1, in);
  vb_.SetAttribs(shader_.GetAttribLocation("a_position"),
                 shader_.GetAttribLocation("a_texcoord0"),
                 shader_.GetAttribLocation("a_texcoord1"));
  out.BindFramebuffer();
  glViewport(0, 0, w/2+1, h);
  vb_.Draw();
  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}  // namespace gpu
