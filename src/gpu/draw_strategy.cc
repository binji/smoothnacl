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
  InitShader();
}

void DrawStrategy::Draw(SimulationThreadDrawOptions options,
                        SimulationBase* simulation) {
  Simulation* gpu_sim = static_cast<Simulation*>(simulation);

  locked_queue_->PushBack(g_task_list.Take());

  // view.cc sets the viewport to its full size before it runs commands from
  // the task list. Ensure the draw call is the first render we perform for
  // this frame.
  switch (options) {
    default:
    case kDrawOptions_Simulation:
      Apply(gpu_sim->aa());
      break;

    case kDrawOptions_KernelDisc:
      Apply(gpu_sim->kernel().kd());
      break;

    case kDrawOptions_KernelRing:
      Apply(gpu_sim->kernel().kr());
      break;

    case kDrawOptions_Smoother:
      gpu_sim->ViewSmoother();
      Apply(gpu_sim->aa());
      break;
  }
}

void DrawStrategy::InitShader() {
  shader_.Init(shader_source_draw_frag, shader_source_1tex_vert);
  vb_.SetSize(1, 1);
  vb_.SetTex(0, 0, 0, 1, 1);
  vb_.LoadData();
}

void DrawStrategy::Apply(const Texture& in) {
  int w = size_.width();
  int h = size_.height();
  shader_.Use();
  shader_.UniformMatrixOrtho("u_mat", 0, 1, 1, 0, -1, 1);
  shader_.UniformTexture("u_tex0", 0, in);
  vb_.SetAttribs(shader_.GetAttribLocation("a_position"),
                 shader_.GetAttribLocation("a_texcoord0"));
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  vb_.Draw();
  glUseProgram(0);
}

}  // namespace gpu
