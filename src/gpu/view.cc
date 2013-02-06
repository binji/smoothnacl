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

#include "gpu/view.h"
#include <GLES2/gl2.h>
#include <stdio.h>
#include <ppapi/cpp/graphics_3d.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/lib/gl/gles2/gl2ext_ppapi.h>
#include "gpu/gl_task.h"

#define printf(...)

namespace gpu {

View::View(const pp::Size& sim_size, LockedQueue* locked_queue)
    : factory_(this),
      graphics_3d_(NULL),
      sim_size_(sim_size),
      draw_loop_running_(false),
      locked_queue_(locked_queue) {
}

View::~View() {
  delete graphics_3d_;
}

bool View::DidChangeView(pp::Instance* instance, const pp::View& view) {
  pp::Size old_size = size_;
  pp::Size new_size = view.GetRect().size();
  if (old_size == new_size)
    return true;

  printf("Size: %d x %d\n", new_size.width(), new_size.height());

  int32_t result;

  if (!graphics_3d_) {
    int32_t attribs[] = {
      PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 8,
      PP_GRAPHICS3DATTRIB_SAMPLES, 0,
      PP_GRAPHICS3DATTRIB_SAMPLE_BUFFERS, 0,
      PP_GRAPHICS3DATTRIB_WIDTH, new_size.width(),
      PP_GRAPHICS3DATTRIB_HEIGHT, new_size.height(),
      PP_GRAPHICS3DATTRIB_NONE
    };

    graphics_3d_ = new pp::Graphics3D(instance, attribs);

    if (!instance->BindGraphics(*graphics_3d_)) {
      delete graphics_3d_;
      graphics_3d_ = NULL;
      size_ = pp::Size();
      return false;
    }
  } else {
    result = graphics_3d_->ResizeBuffers(new_size.width(), new_size.height());
    if (result != PP_OK) {
      delete graphics_3d_;
      graphics_3d_ = NULL;
      size_ = pp::Size();
      return false;
    }
  }

  glSetCurrentContextPPAPI(graphics_3d_->pp_resource());

  if (!draw_loop_running_) {
    SwapBuffersCallback(0);  // Start the render loop.
    draw_loop_running_ = true;
  }

  size_ = new_size;

  return true;
}

pp::Size View::GetSize() const {
  return size_;
}

void View::SwapBuffersCallback(int32_t result) {
  if (!graphics_3d_) {
    draw_loop_running_ = false;
    return;
  }

  double scale;
  int xoffset, yoffset;
  GetScreenToSimScale(sim_size_, &scale, &xoffset, &yoffset);
  int vp_width = static_cast<int>(sim_size_.width() / scale);
  int vp_height = static_cast<int>(sim_size_.height() / scale);

  // Run all commands from the front of the task queue.
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(0.5, 0.5, 0.5, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(xoffset, yoffset, vp_width, vp_height);
  locked_queue_->PopFront().RunAndClear();

  graphics_3d_->SwapBuffers(factory_.NewCallback(&View::SwapBuffersCallback));
}

}  // namespace gpu
