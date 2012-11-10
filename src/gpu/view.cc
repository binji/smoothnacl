// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/view.h"
#include <GLES2/gl2.h>
#include <stdio.h>
#include <ppapi/cpp/graphics_3d.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/lib/gl/gles2/gl2ext_ppapi.h>
#include "gpu/gl_task.h"

namespace gpu {

View::View(LockedQueue* locked_queue)
    : factory_(this),
      graphics_3d_(NULL),
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

  // Run all commands from the front of the task queue.
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(0.5, 0.5, 0.5, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  locked_queue_->PopFront().RunAndClear();

  graphics_3d_->SwapBuffers(factory_.NewCallback(&View::SwapBuffersCallback));
}

}  // namespace gpu
