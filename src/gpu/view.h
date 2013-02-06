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

  View(const View&);  // undefined
  View& operator =(const View&);  // undefined
};

}  // namespace gpu

#endif  // GPU_VIEW_H_
