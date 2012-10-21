// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "smoothlife_view.h"

#include <math.h>
#include <pthread.h>
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

namespace {

const uint32_t kBlack = 0xff000000;
const uint32_t kWhite = 0xffffffff;

pp::Rect ClipRect(const pp::Rect& rect, const pp::Rect& clip) {
  return clip.Intersect(rect);
}

class ScopedMutexLock {
 public:
  explicit ScopedMutexLock(pthread_mutex_t* mutex) : mutex_(mutex) {
    if (pthread_mutex_lock(mutex_) != 0) {
      mutex_ = NULL;
    }
  }
  ~ScopedMutexLock() {
    if (mutex_)
      pthread_mutex_unlock(mutex_);
  }
  bool is_valid() const {
    return mutex_ != NULL;
  }
 private:
  pthread_mutex_t* mutex_;  // Weak reference.
};

// A small helper RAII class used to acquire and release the pixel lock.
class ScopedPixelLock {
 public:
  explicit ScopedPixelLock(SmoothlifeView* image_owner)
      : image_owner_(image_owner),
        pixels_(image_owner->LockPixels()) {}

  ~ScopedPixelLock() {
    pixels_ = NULL;
    image_owner_->UnlockPixels();
  }

  uint32_t* pixels() const {
    return pixels_;
  }

 private:
  SmoothlifeView* image_owner_;  // Weak reference.
  uint32_t* pixels_;  // Weak reference.

  ScopedPixelLock();  // Not implemented, do not use.
};

}  // namespace

SmoothlifeView::SmoothlifeView()
    : factory_(this),
      graphics_2d_(NULL),
      pixel_buffer_(NULL),
      quit_(false) {
  pthread_mutex_init(&pixel_buffer_mutex_, NULL);
  thread_create_result_ = pthread_create(&thread_, NULL, &SmoothlifeThread,
                                         this);
}

SmoothlifeView::~SmoothlifeView() {
  quit_ = true;
  if (thread_create_result_ == 0)
    pthread_join(thread_, NULL);
  pthread_mutex_destroy(&pixel_buffer_mutex_);
  delete graphics_2d_;
  delete pixel_buffer_;
}

bool SmoothlifeView::DidChangeView(pp::Instance* instance,
                             const pp::View& view,
                             bool first_view_change) {
  pp::Size old_size = GetSize();
  pp::Size new_size = view.GetRect().size();
  if (old_size == new_size)
    return true;

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

  if (first_view_change)
    DrawCallback(0);  // Start the draw loop.

  return true;
}

pp::Size SmoothlifeView::GetSize() const {
  return graphics_2d_ ? graphics_2d_->size() : pp::Size();
}

void SmoothlifeView::DrawCallback(int32_t result) {
  assert(graphics_2d_);
  assert(pixel_buffer_);
  ScopedMutexLock lock(&pixel_buffer_mutex_);

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

uint32_t* SmoothlifeView::LockPixels() {
  void* pixels = NULL;
  // Do not use a ScopedMutexLock here, since the lock needs to be held until
  // the matching UnlockPixels() call.
  if (pthread_mutex_lock(&pixel_buffer_mutex_) == 0) {
    if (pixel_buffer_ != NULL && !pixel_buffer_->is_null()) {
      pixels = pixel_buffer_->data();
    }
  }
  return reinterpret_cast<uint32_t*>(pixels);
}

void SmoothlifeView::UnlockPixels() {
  pthread_mutex_unlock(&pixel_buffer_mutex_);
}

//0 12.0 3.0 12.0 0.100 0.278 0.365 0.267 0.445 4 4 4 0.028 0.147
#if 0
const int mode = 0;
const double ra = 12.0;
const double rr = 3.0;
const double rb = 12.0;
const double dt = 0.100;
const double b1 = 0.278;
const double b2 = 0.365;
const double d1 = 0.267;
const double d2 = 0.445;
const int sigmode = 4;
const int sigtype = 4;
const int mixtype = 4;
const double sn = 0.028;
const double sm = 0.147;
#elif 1
//1 31.8 3.0 31.8 0.157 0.092 0.098 0.256 0.607 4 4 4 0.015 0.340
//1 21.8 3.0 21.8 0.157 0.192 0.200 0.355 0.600 4 4 4 0.025 0.490
//1 21.8 3.0 21.8 0.157 0.232 0.337 0.599 0.699 4 4 4 0.025 0.290
const int mode = 1;
double ra = 21.8;
double rr = 3.0;
double rb = 21.8;
double dt = 0.157;
double b1 = 0.192;
double b2 = 0.200;
double d1 = 0.355;
double d2 = 0.600;
int sigmode = 4;
int sigtype = 4;
int mixtype = 4;
double sn = 0.025;
double sm = 0.490;
#else
//2 12.0 3.0 12.0 0.115 0.269 0.340 0.523 0.746 4 4 4 0.028 0.147
//2 12.0 3.0 12.0 0.415 0.269 0.350 0.513 0.756 4 4 4 0.028 0.147
const int mode = 2;
const double ra = 12.0;
const double rr = 3.0;
const double rb = 12.0;
const double dt = 0.115;
const double b1 = 0.269;
const double b2 = 0.340;
const double d1 = 0.523;
const double d2 = 0.746;
const int sigmode = 4;
const int sigtype = 4;
const int mixtype = 4;
const double sn = 0.028;
const double sm = 0.147;
#endif


#if 0
double RND(double x) {
  return x * (double)rand()/((double)RAND_MAX + 1);
}

void inita2D(double* a) {
  double mx, my;

  mx = 2*ra; if (mx>NX) mx = NX;
  my = 2*ra; if (my>NY) my = NY;

  for (int t = 0; t<= (int)(NX*NY/(mx*my)); t++) {
//  for (int t = 0; t<=15; t++) {
    splat2D(a);
  }
}

#endif

void SmoothlifeView::DrawBuffer(const FftAllocation<double>& a) {
  ScopedPixelLock lock(this);
  uint32_t* pixels = lock.pixels();
  if (!pixels)
    return;

#if 1
  int image_width = GetSize().width();
  int image_height = GetSize().height();
  int buffer_width = a.size().width();
  int buffer_height = a.size().height();

  for (int y = 0; y < image_height; ++y) {
    int buffer_y = buffer_height * y / image_height;
    for (int x = 0; x < image_width; ++x) {
      // Cheesy scaling.
      int buffer_x = buffer_width * x / image_width;
      double dv = a[buffer_y * buffer_width + buffer_x];
      uint8_t v = 255 * dv; //255 * (1 - dv);
      uint32_t color = 0xff000000 | (v<<16) | (v<<8) | v;
      pixels[y * image_width + x] = color;
    }
  }
#else
  int image_width = GetSize().width();
  int image_height = GetSize().height();
  for (int y = 0; y < image_height; ++y) {
    for (int x = 0; x < image_width; ++x) {
      // Cheesy scaling.
      double dv = a[y * image_width + x];
      uint8_t v = 255 * dv; //255 * (1 - dv);
      uint32_t color = 0xff000000 | (v<<16) | (v<<8) | v;
      pixels[y * image_width + x] = color;
    }
  }
#endif
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

void* SmoothlifeView::SmoothlifeThread(void* param) {
  SmoothlifeView* self = static_cast<SmoothlifeView*>(param);

  pp::Size sim_size(512, 512);
  KernelConfig kernel_config;
  kernel_config.ra = 12.0;
  kernel_config.rr = 3.0;
  kernel_config.rb = 12.0;
  SmootherConfig smoother_config;
  smoother_config.timestep.type = TIMESTEP_DISCRETE;
  smoother_config.timestep.dt = 0.100;
  smoother_config.b1 = 0.278;
  smoother_config.b2 = 0.365;
  smoother_config.d1 = 0.267;
  smoother_config.d2 = 0.445;
  smoother_config.mode = SIGMOID_MODE_4;
  smoother_config.sigmoid = SIGMOID_SMOOTH;
  smoother_config.mix = SIGMOID_SMOOTH;
  smoother_config.sn = 0.028;
  smoother_config.sm = 0.147;
  Simulation simulation(sim_size, kernel_config, smoother_config);
  simulation.Clear(0);
  pp::Size size = self->GetSize();
//  int w = size.width();
//  int h = size.height();
//  simulation.DrawFilledCircle(w / 2, h / 2, std::min(w, h) / 3, 1.0);
  simulation.inita2D(kernel_config.ra);

  while (!self->quit_) {
    self->DrawBuffer(simulation.buffer());
    simulation.Step();
  }

  return NULL;
}
