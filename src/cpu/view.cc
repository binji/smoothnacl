// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cpu/view.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/graphics_2d.h>
#include <ppapi/cpp/image_data.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/instance_handle.h>
#include <ppapi/cpp/point.h>
#include "palette.h"


namespace cpu {

namespace {

bool IsPowerOf2(uint32_t x) {
  return x && (x & (x - 1)) == 0;
}

}  // namespace

View::View(LockedObject<AlignedUint32>* buffer)
    : factory_(this),
      graphics_2d_(NULL),
      pixel_buffer_(NULL),
      locked_buffer_(buffer),
      draw_loop_running_(false) {
}

View::~View() {
  delete graphics_2d_;
  delete pixel_buffer_;
}

bool View::DidChangeView(pp::Instance* instance,
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

pp::Size View::GetSize() const {
  return graphics_2d_ ? graphics_2d_->size() : pp::Size();
}

void View::DrawCallback(int32_t result) {
  if (!graphics_2d_) {
    draw_loop_running_ = false;
    return;
  }
  assert(pixel_buffer_);

  AlignedUint32* data = locked_buffer_->Lock();
  DrawBuffer(*data);
  locked_buffer_->Unlock();

  PaintRectToGraphics2D(pp::Rect(GetSize()));

  // Graphics2D::Flush writes all paints to the graphics context's backing
  // store. When it is finished, it calls the callback. By hooking our draw
  // function to the Flush callback, we will be able to draw as quickly as
  // possible.
  graphics_2d_->Flush(factory_.NewCallback(&View::DrawCallback));
}

void View::PaintRectToGraphics2D(const pp::Rect& rect) {
  const pp::Point top_left(0, 0);
  graphics_2d_->PaintImageData(*pixel_buffer_, top_left, rect);
}

void View::DrawBuffer(const AlignedUint32& a) {
  uint32_t* pixels = static_cast<uint32_t*>(pixel_buffer_->data());
  if (!pixels)
    return;

  double scale;
  int x_offset;
  int y_offset;
  GetScreenToSimScale(a.size(), &scale, &x_offset, &y_offset);

  int image_width = GetSize().width();
  int image_height = GetSize().height();
  int buffer_width = a.size().width();
  int buffer_height = a.size().height();

  assert(IsPowerOf2(buffer_width));
  assert(IsPowerOf2(buffer_height));

  for (int y = 0; y < image_height; ++y) {
    int buffer_y = static_cast<int>((y - y_offset) * scale);
    buffer_y = (buffer_y + buffer_height) & (buffer_height - 1);
    for (int x = 0; x < image_width; ++x) {
      int buffer_x = static_cast<int>((x - x_offset) * scale);
      buffer_x = (buffer_x + buffer_width) & (buffer_width - 1);
      pixels[y * image_width + x] = a[buffer_y * buffer_width + buffer_x];
    }
  }
}

}  // namespace cpu
