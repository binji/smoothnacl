// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CPU_VIEW_H_
#define CPU_VIEW_H_

#include <ppapi/cpp/point.h>
#include <ppapi/cpp/rect.h>
#include <ppapi/utility/completion_callback_factory.h>
#include "fft_allocation.h"
#include "locked_object.h"

namespace pp {
class Graphics2D;
class ImageData;
class Instance;
class Size;
class View;
}  // namespace pp

namespace cpu {

class View {
 public:
  explicit View(LockedObject<AlignedReals>* buffer);
  ~View();

  bool DidChangeView(pp::Instance* instance, const pp::View& view);
  pp::Size GetSize() const;
  pp::Point ScreenToSim(const pp::Point& p, const pp::Size& sim_size) const;

 private:
  void GetScreenToSimScale(const pp::Size& sim_size, double* out_scale,
                           int* out_xoffset, int* out_yoffset) const;
  void DrawCallback(int32_t result);
  void DrawRect(const pp::Rect& rect, uint32_t color);
  void PaintRectToGraphics2D(const pp::Rect& rect);
  void DrawBuffer(const FftAllocation<double>& a);

  pp::CompletionCallbackFactory<View> factory_;
  pp::Graphics2D* graphics_2d_;
  pp::ImageData* pixel_buffer_;
  LockedObject<AlignedReals>* locked_buffer_;  // Weak.
  bool draw_loop_running_;
};

}  // namespace cpu

#endif  // CPU_VIEW_H_
