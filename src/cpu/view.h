// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
