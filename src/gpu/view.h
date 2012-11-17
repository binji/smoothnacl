// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_VIEW_H_
#define GPU_VIEW_H_

#include <ppapi/cpp/point.h>
#include <ppapi/cpp/size.h>
#include <ppapi/utility/completion_callback_factory.h>
#include "gpu/locked_queue.h"
#include "view_base.h"

namespace pp {
class Graphics3D;
class Instance;
class View;
}  // namespace pp

namespace gpu {

class GLTaskList;

class View : public ViewBase {
 public:
  View(const pp::Size& sim_size, LockedQueue* locked_queue);
  ~View();

  virtual bool DidChangeView(pp::Instance* instance, const pp::View& view);
  virtual pp::Size GetSize() const;

 private:
  void SwapBuffersCallback(int32_t result);

  pp::CompletionCallbackFactory<View> factory_;
  pp::Graphics3D* graphics_3d_;
  pp::Size size_;
  pp::Size sim_size_;
  bool draw_loop_running_;
  LockedQueue* locked_queue_;  // Weak.
};

}  // namespace gpu

#endif  // GPU_VIEW_H_
