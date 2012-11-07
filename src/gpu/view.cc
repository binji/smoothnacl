// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/view.h"
#include <GLES2/gl2.h>
#include <stdio.h>
#include <ppapi/cpp/graphics_3d.h>
#include <ppapi/cpp/instance.h>
#include "gpu/gl_task.h"

namespace gpu {

View::View(LockedObject<GLTaskList>* locked_tasks)
    : factory_(this),
      graphics_3d_(NULL),
      draw_loop_running_(false),
      locked_tasks_(locked_tasks) {
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
  } else {
    result = graphics_3d_->ResizeBuffers(new_size.width(), new_size.height());
    if (result != PP_OK) {
      delete graphics_3d_;
      graphics_3d_ = NULL;
      size_ = pp::Size();
      return false;
    }
  }

  if (!instance->BindGraphics(*graphics_3d_)) {
    delete graphics_3d_;
    graphics_3d_ = NULL;
    size_ = pp::Size();
    return false;
  }

  if (!draw_loop_running_) {
    SwapBuffersCallback(0);  // Start the render loop.
    draw_loop_running_ = true;
  }

  size_ = new_size;

  return true;
}

pp::Point View::ScreenToSim(const pp::Point& p,
                            const pp::Size& sim_size) const {
  double scale;
  int x_offset;
  int y_offset;
  GetScreenToSimScale(sim_size, &scale, &x_offset, &y_offset);
  return pp::Point(
      static_cast<int>((p.x() - x_offset) * scale),
      static_cast<int>((p.y() - y_offset) * scale));
}


void View::GetScreenToSimScale(const pp::Size& sim_size, double* out_scale,
                               int* out_xoffset, int* out_yoffset) const {
  // Keep the aspect ratio.
  int image_width = size_.width();
  int image_height = size_.height();
  *out_scale = std::max(
      static_cast<double>(sim_size.width()) / image_width,
      static_cast<double>(sim_size.height()) / image_height);
  *out_xoffset =
      static_cast<int>((image_width - sim_size.width() / *out_scale) / 2);
  *out_yoffset =
      static_cast<int>((image_height - sim_size.height() / *out_scale) / 2);
}

void View::SwapBuffersCallback(int32_t result) {
  if (!graphics_3d_) {
    draw_loop_running_ = false;
    return;
  }

  GLTaskList* task_list = locked_tasks_->Lock();
  GLTaskList tasks = task_list->Take();
  locked_tasks_->Unlock();
  tasks.RunAndClear();

  graphics_3d_->SwapBuffers(factory_.NewCallback(&View::SwapBuffersCallback));
}

}  // namespace gpu
