// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/draw_strategy.h"
#include "gen/shader_source.h"
#include "gpu/gl_task.h"
#include "gpu/simulation.h"
#include "gpu/texture.h"
#include "gpu/wrap_gl.h"

namespace gpu {

DrawStrategy::DrawStrategy(const pp::Size& size,
                           LockedQueue* locked_queue)
    : size_(size),
      locked_queue_(locked_queue) {
}

void DrawStrategy::Draw(ThreadDrawOptions options, SimulationBase* simulation) {
  Simulation* gpu_sim = static_cast<Simulation*>(simulation);

  switch (options) {
    default:
    case kDrawOptions_Simulation:
      Apply(gpu_sim->aa());
      break;

    case kDrawOptions_KernelDisc:
      // TODO(binji): implement.
      break;

    case kDrawOptions_KernelRing:
      // TODO(binji): implement.
      break;

    case kDrawOptions_Smoother:
      // TODO(binji): implement.
      break;
  }

  locked_queue_->PushBack(g_task_list.Take());
}

void DrawStrategy::InitShader() {
  shader_.Init(shader_source_draw_frag, shader_source_draw_vert);
  vb_.SetSize(1, 1);
  vb_.SetTex(0, 0, 0, 1, 1);
  vb_.LoadData();
}

void DrawStrategy::Apply(const Texture& in) {
  int w = size_.width();
  int h = size_.height();
  shader_.Use();
  shader_.UniformMatrixOrtho("u_mat", 0, 1, 1, 0, -1, 1);
  shader_.UniformTexture("tex0", 0, in);
  shader_.Uniform1f("colscheme", 2);  // TODO(binji): real value...?
  shader_.Uniform1f("phase", 0);  // TODO(binji): real value...?
  vb_.SetAttribs(shader_.GetAttribLocation("a_position"),
                 shader_.GetAttribLocation("a_texcoord0"));
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, w, h);  // TODO(binji): get real w/h from view...?
  vb_.Draw();
  glUseProgram(0);
}

}  // namespace gpu
