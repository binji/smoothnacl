// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_VIEW_H_
#define GPU_VIEW_H_

#include <ppapi/cpp/point.h>
#include <ppapi/cpp/size.h>
#include <ppapi/utility/completion_callback_factory.h>
#include "locked_object.h"

namespace pp {
class Graphics3D;
class Instance;
class View;
}  // namespace pp

namespace gpu {

class GLTaskList;

class View {
 public:
  explicit View(LockedObject<GLTaskList>* locked_tasks);
  ~View();

  bool DidChangeView(pp::Instance* instance, const pp::View& view);
  pp::Point ScreenToSim(const pp::Point& p, const pp::Size& sim_size) const;

 private:
  void GetScreenToSimScale(const pp::Size& sim_size, double* out_scale,
                           int* out_xoffset, int* out_yoffset) const;
  void SwapBuffersCallback(int32_t result);

  pp::CompletionCallbackFactory<View> factory_;
  pp::Graphics3D* graphics_3d_;
  pp::Size size_;
  bool draw_loop_running_;
  LockedObject<GLTaskList>* locked_tasks_;  // Weak.
};

}  // namespace gpu

#endif  // GPU_VIEW_H_
