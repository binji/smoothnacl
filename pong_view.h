// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXAMPLES_PONG_VIEW_H_
#define EXAMPLES_PONG_VIEW_H_

#include "ppapi/cpp/rect.h"
#include "ppapi/utility/completion_callback_factory.h"

namespace pp {
class Graphics2D;
class ImageData;
class Instance;
class Size;
class View;
}  // namespace pp

class PongView {
 public:
  PongView();
  ~PongView();

  bool DidChangeView(pp::Instance* instance, const pp::View& view,
                     bool first_view_change);
  pp::Size GetSize() const;
  void StartDrawLoop();
  uint32_t* LockPixels();
  void UnlockPixels();

 private:
  void DrawCallback(int32_t result);
  void DrawRect(const pp::Rect& rect, uint32_t color);
  void PaintRectToGraphics2D(const pp::Rect& rect);
  static void* SmoothlifeThread(void* param);
  void DrawBuffer(double* a);

  pp::CompletionCallbackFactory<PongView> factory_;
  pp::Graphics2D* graphics_2d_;
  pp::ImageData* pixel_buffer_;
  pthread_t thread_;
  int thread_create_result_;
  pthread_mutex_t pixel_buffer_mutex_;
  bool quit_;
};

#endif  // EXAMPLES_PONG_VIEW_H_
