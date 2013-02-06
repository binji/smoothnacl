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

#ifndef CPU_VIEW_H_
#define CPU_VIEW_H_

#include <ppapi/c/pp_time.h>
#include <ppapi/cpp/point.h>
#include <ppapi/cpp/rect.h>
#include <ppapi/utility/completion_callback_factory.h>
#include "fft_allocation.h"
#include "locked_object.h"
#include "view_base.h"

class Palette;

namespace pp {
class Graphics2D;
class ImageData;
class Instance;
class Size;
class View;
}  // namespace pp

namespace cpu {

class View : public ViewBase {
 public:
  explicit View(LockedObject<AlignedUint32>* buffer);
  ~View();

  virtual bool DidChangeView(pp::Instance* instance, const pp::View& view);
  virtual pp::Size GetSize() const;

 private:
  void DrawCallback(int32_t result);
  void DrawRect(const pp::Rect& rect, uint32_t color);
  void PaintRectToGraphics2D(const pp::Rect& rect);
  void DrawBuffer(const AlignedUint32& a);

  pp::CompletionCallbackFactory<View> factory_;
  pp::Graphics2D* graphics_2d_;
  pp::ImageData* pixel_buffer_;
  LockedObject<AlignedUint32>* locked_buffer_;  // Weak.
  bool draw_loop_running_;
};

}  // namespace cpu

#endif  // CPU_VIEW_H_
