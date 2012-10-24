// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXAMPLES_SMOOTHLIFE_VIEW_H_
#define EXAMPLES_SMOOTHLIFE_VIEW_H_

#include "ppapi/cpp/rect.h"
#include "ppapi/utility/completion_callback_factory.h"
#include "fft_allocation.h"
#include "locked_object.h"

namespace pp {
class Graphics2D;
class ImageData;
class Instance;
class Size;
class View;
}  // namespace pp

class SmoothlifeView {
 public:
  SmoothlifeView(LockedObject<AlignedReals>* buffer);
  ~SmoothlifeView();

  bool DidChangeView(pp::Instance* instance, const pp::View& view);
  pp::Size GetSize() const;
  void StartDrawLoop();
  uint32_t* LockPixels();
  void UnlockPixels();

 private:
  void DrawCallback(int32_t result);
  void DrawRect(const pp::Rect& rect, uint32_t color);
  void PaintRectToGraphics2D(const pp::Rect& rect);
  void DrawBuffer(const FftAllocation<double>& a);

  pp::CompletionCallbackFactory<SmoothlifeView> factory_;
  pp::Graphics2D* graphics_2d_;
  pp::ImageData* pixel_buffer_;
  LockedObject<AlignedReals>* locked_buffer_;  // Weak.
  bool draw_loop_running_;
};

#endif  // EXAMPLES_SMOOTHLIFE_VIEW_H_
