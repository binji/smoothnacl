// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "smoothlife_view.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/graphics_2d.h"
#include "ppapi/cpp/image_data.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/instance_handle.h"
#include "ppapi/cpp/point.h"

#include "kernel.h"
#include "simulation.h"
#include "smoother.h"

SmoothlifeView::SmoothlifeView(LockedObject<AlignedReals>* buffer)
    : factory_(this),
      graphics_2d_(NULL),
      pixel_buffer_(NULL),
      locked_buffer_(buffer),
      draw_loop_running_(false) {
}

SmoothlifeView::~SmoothlifeView() {
  delete graphics_2d_;
  delete pixel_buffer_;
}

bool SmoothlifeView::DidChangeView(pp::Instance* instance,
                             const pp::View& view) {
  pp::Size old_size = GetSize();
  pp::Size new_size = view.GetRect().size();
  if (old_size == new_size)
    return true;

  printf("Size: %d x %d\n", new_size.width(), new_size.height());

  delete graphics_2d_;
  graphics_2d_ = new pp::Graphics2D(instance, new_size,
                                    true);  // is_always_opaque
  if (!instance->BindGraphics(*graphics_2d_)) {
    delete graphics_2d_;
    graphics_2d_ = NULL;
    return false;
  }

  // Create a new pixel buffer, the same size as the graphics context. We'll
  // write to this buffer directly, and copy regions of it to the graphics
  // context's backing store to draw to the screen.
  delete pixel_buffer_;
  pixel_buffer_ = new pp::ImageData(instance, PP_IMAGEDATAFORMAT_BGRA_PREMUL,
                                    new_size,
                                    true);  // init_to_zero

  if (!draw_loop_running_) {
    DrawCallback(0);  // Start the draw loop.
    draw_loop_running_ = true;
  }

  return true;
}

pp::Size SmoothlifeView::GetSize() const {
  return graphics_2d_ ? graphics_2d_->size() : pp::Size();
}

void SmoothlifeView::DrawCallback(int32_t result) {
  if (!graphics_2d_) {
    draw_loop_running_ = false;
    return;
  }
  assert(pixel_buffer_);

  AlignedReals* data = locked_buffer_->Lock();
  DrawBuffer(*data);
  locked_buffer_->Unlock();

  PaintRectToGraphics2D(pp::Rect(GetSize()));

  // Graphics2D::Flush writes all paints to the graphics context's backing
  // store. When it is finished, it calls the callback. By hooking our draw
  // function to the Flush callback, we will be able to draw as quickly as
  // possible.
  graphics_2d_->Flush(factory_.NewCallback(&SmoothlifeView::DrawCallback));
}

void SmoothlifeView::PaintRectToGraphics2D(const pp::Rect& rect) {
  const pp::Point top_left(0, 0);
  graphics_2d_->PaintImageData(*pixel_buffer_, top_left, rect);
}

void SmoothlifeView::DrawBuffer(const AlignedReals& a) {
  uint32_t* pixels = static_cast<uint32_t*>(pixel_buffer_->data());
  if (!pixels)
    return;

  int image_width = GetSize().width();
  int image_height = GetSize().height();
  int buffer_width = a.size().width();
  int buffer_height = a.size().height();

  // Keep the aspect ratio.
  double min_scale = std::max(
      static_cast<double>(buffer_width) / image_width,
      static_cast<double>(buffer_height) / image_height);
  int x_offset = (image_width - buffer_width / min_scale) / 2;
  int y_offset = (image_height - buffer_height / min_scale) / 2;

  double buffer_x = 0;
  double buffer_y = 0;

  for (int y = y_offset; y < image_height - y_offset; ++y) {
    buffer_x = 0;
    for (int x = x_offset; x < image_width - x_offset; ++x) {
      double dv = a[(int)buffer_y * buffer_width + (int)buffer_x];
      uint8_t v = 255 * dv; //255 * (1 - dv);
      uint32_t color = 0xff000000 | (v<<16) | (v<<8) | v;
      pixels[y * image_width + x] = color;
      buffer_x += min_scale;
    }
    buffer_y += min_scale;
  }
}


#if 0
unsigned long long rdtsc(void) {
  unsigned int tickl, tickh;
  __asm__ __volatile__("rdtsc":"=a"(tickl),"=d"(tickh));
  return ((unsigned long long)tickh << 32)|tickl;
}

#if 1
  const int kCounter = 100;
  int counter = kCounter;
  const char* name[10];
  uint64_t tsc[10];
  memset(&tsc[0], 0, sizeof(uint64_t) * 10);
#endif

#define MARK(X) do { \
    name[c] = #X; \
    X; \
    temp = rdtsc(); \
    tsc[c++] += temp - last; \
    last = temp; \
  } while(0)

  if (--counter == 0) {
    for (int i = 0; i < c; ++i) {
      printf("%s: %g\n", name[i], static_cast<double>(tsc[i]) / kCounter);
    }
    counter = kCounter;
    memset(&tsc[0], 0, sizeof(uint64_t) * 10);
  }
#endif

