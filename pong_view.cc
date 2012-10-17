// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pong_view.h"

#include <fftw3.h>
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
  explicit ScopedPixelLock(PongView* image_owner)
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
  PongView* image_owner_;  // Weak reference.
  uint32_t* pixels_;  // Weak reference.

  ScopedPixelLock();  // Not implemented, do not use.
};

}  // namespace

PongView::PongView()
    : factory_(this),
      graphics_2d_(NULL),
      pixel_buffer_(NULL),
      quit_(false) {
  pthread_mutex_init(&pixel_buffer_mutex_, NULL);
  thread_create_result_ = pthread_create(&thread_, NULL, &SmoothlifeThread,
                                         this);
}

PongView::~PongView() {
  quit_ = true;
  if (thread_create_result_ == 0)
    pthread_join(thread_, NULL);
  pthread_mutex_destroy(&pixel_buffer_mutex_);
  delete graphics_2d_;
  delete pixel_buffer_;
}

bool PongView::DidChangeView(pp::Instance* instance,
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

pp::Size PongView::GetSize() const {
  return graphics_2d_ ? graphics_2d_->size() : pp::Size();
}

void PongView::DrawCallback(int32_t result) {
  assert(graphics_2d_);
  assert(pixel_buffer_);
  ScopedMutexLock lock(&pixel_buffer_mutex_);

  PaintRectToGraphics2D(pp::Rect(GetSize()));

  // Graphics2D::Flush writes all paints to the graphics context's backing
  // store. When it is finished, it calls the callback. By hooking our draw
  // function to the Flush callback, we will be able to draw as quickly as
  // possible.
  graphics_2d_->Flush(factory_.NewCallback(&PongView::DrawCallback));
}

void PongView::PaintRectToGraphics2D(const pp::Rect& rect) {
  const pp::Point top_left(0, 0);
  graphics_2d_->PaintImageData(*pixel_buffer_, top_left, rect);
}

uint32_t* PongView::LockPixels() {
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

void PongView::UnlockPixels() {
  pthread_mutex_unlock(&pixel_buffer_mutex_);
}

//0 12.0 3.0 12.0 0.100 0.278 0.365 0.267 0.445 4 4 4 0.028 0.147
//1 31.8 3.0 31.8 0.157 0.092 0.098 0.256 0.607 4 4 4 0.015 0.340
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



const double PI = 6.28318530718;
const int NX = 512;
const int NY = 512;
double kflr = 0;
double kfld = 0;

double* AllocateReal() {
  const int kRealSize = NX * NY;
  return static_cast<double*>(fftw_malloc(sizeof(double) * kRealSize));
}

fftw_complex* AllocateComplex() {
  const int kComplexSize = NX * (NY / 2 + 1);
  return static_cast<fftw_complex*>(
      fftw_malloc(sizeof(fftw_complex*) * kComplexSize));
}

double func_hard (double x, double a) {
  if (x>=a) return 1.0; else return 0.0;
}

double func_linear (double x, double a, double ea) {
  if (x < a-ea/2.0) return 0.0;
  else if (x > a+ea/2.0) return 1.0;
  else return (x-a)/ea + 0.5;
}

double func_hermite (double x, double a, double ea) {
  if (x < a-ea/2.0) return 0.0;
  else if (x > a+ea/2.0) return 1.0;
  else {
    double m = (x-(a-ea/2.0))/ea;
    return m*m*(3.0-2.0*m);
  }
}

double func_sin (double x, double a, double ea) {
  if (x < a-ea/2.0) return 0.0;
  else if (x > a+ea/2.0) return 1.0;
  else return sin(PI/2.0*(x-a)/ea)*0.5+0.5;
}

double func_smooth (double x, double a, double ea) {
  return 1.0/(1.0+exp(-(x-a)*4.0/ea));
}

double func_kernel (double x, double a, double ea) {
  //return func_hard   (x, a);
  return func_linear  (x, a, ea);
  //return func_hermite (x, a, ea);
  //return func_sin     (x, a, ea);
  //return func_smooth  (x, a, ea);
}

void makekernel(double* kr, double* kd) {
  int ix, iy, x, y, z;
  double l, n, m;
  int Ra;
  double ri, bb;

  ri = ra/rr;
  bb = ra/rb;

  //Ra = (int)(ra+bb/2+1.0);
  Ra = (int)(ra*2);

  kflr = 0.0;
  kfld = 0.0;

  memset(kd, 0, NX*NY*sizeof(double));
  memset(kr, 0, NX*NY*sizeof(double));
  z = 0;
  for (ix=0; ix<NX; ix++) {
    x = (ix < NX/2) ? ix : ix - NX;
    if (x>=-Ra && x<=Ra) {
      for (iy=0; iy<NY; iy++) {
        y = (iy < NY / 2) ? iy : iy - NY;
        if (y>=-Ra && y<=Ra) {
          l = sqrt (x*x + y*y + z*z);
          m = 1 - func_kernel(l, ri, bb);
          n = func_kernel(l, ri, bb) * (1 - func_kernel(l, ra, bb));
          *(kd + ix*NY + iy) = m;
          *(kr + ix*NY + iy) = n;
          kflr += n;
          kfld += m;
        }
      }
    }
  }
}

void PongView::DrawBuffer(double* a) {
  ScopedPixelLock lock(this);
  uint32_t* pixels = lock.pixels();
  if (!pixels)
    return;

  int width = GetSize().width();

  for (int y = 0; y < NY; ++y) {
    for (int x = 0; x < NX; ++x) {
      double av = *(a + x * NY + y);
      uint8_t v = 255 * (1 - av);
      uint32_t color = 0xff000000 | (v<<16) | (v<<8) | v;
      *(pixels + y * width + x) = color;
    }
  }
}

void kernelmul(fftw_complex* vo, fftw_complex* ke, fftw_complex* na, double sc) {
  for (int x = 0; x < NX; ++x) {
    for (int y = 0; y < NY/2+1; ++y) {
      fftw_complex* vov = vo + x * (NY/2+1) + y;
      fftw_complex* kev = ke + x * (NY/2+1) + y;
      fftw_complex* nav = na + x * (NY/2+1) + y;
      (*kev)[0] *= sc;
      (*kev)[1] *= sc;
      (*nav)[0] = (*vov)[0] * (*kev)[0] - (*vov)[1] * (*kev)[1];
      (*nav)[1] = (*vov)[0] * (*kev)[1] + (*vov)[1] * (*kev)[0];
    }
  }
}

double sigmoid_ab(double x, double a, double b) {
#if 0
  return func_smooth(x, a, sn)*(1.0 - func_smooth(x, b, sn));
#else
  return func_hermite(x, a, sn)*(1.0 - func_hermite(x, b, sn));
#endif
}

double sigmoid_mix(double x, double y, int m) {
#if 0
  return x*(1.0 - func_smooth(m, 0.5, sm)) + y*func_smooth(m, 0.5, sm);
#else
  return x*(1.0 - func_hermite(m, 0.5, sm)) + y*func_hermite(m, 0.5, sm);
#endif
}

void snm(double* an, double* am, double* na) {
  for (int x = 0; x < NX; ++x) {
    for (int y = 0; y < NY; ++y) {
      double anv = *(an + x * NY + y);
      double amv = *(am + x * NY + y);
      double* nav = na + x * NY + y;
#if 1
      double f = sigmoid_ab(anv,
                            sigmoid_mix(b1, d1, amv),
                            sigmoid_mix(b2, d2, amv));
#else
      double f = sigmoid_mix(sigmoid_ab(anv, b1, b2),
                             sigmoid_ab(anv, d1, d2),
                             amv);
#endif

#if 0
      switch (mode) {
        default:
        case 0: break;
        case 1: f = *nav + dt * (2.0 * f - 1.0); break;
        case 2: f = *nav + dt * (f - *nav); break;
        case 3: f = amv + dt * (2.0 * f - 1.0); break;
        case 4: f = amv + dt * (f - amv);
      }
#endif
      *nav = f > 1.0 ? 1.0 : f < 0.0 ? 0.0 : f;
    }
  }
}

double RND (double x) {
  return x * (double)rand()/((double)RAND_MAX + 1);
}

void splat2D(double *buf) {
  double mx, my, dx, dy, u, l;
  int ix, iy;

  mx = RND(NX);
  my = RND(NY);
  u = ra*(RND(0.5) + 0.5);

  for (ix=(int)(mx-u-1); ix<=(int)(mx+u+1); ix++) {
    for (iy=(int)(my-u-1); iy<=(int)(my+u+1); iy++) {
      dx = mx-ix;
      dy = my-iy;
      l = sqrt(dx*dx+dy*dy);
      if (l<u) {
        int px = ix;
        int py = iy;
        while (px<  0) px+=NX;
        while (px>=NX) px-=NX;
        while (py<  0) py+=NY;
        while (py>=NY) py-=NY;
        if (px>=0 && px<NX && py>=0 && py<NY) {
          *(buf + NY*px + py) = 1.0;
        }
      }
    }
  }
}


void inita2D(double* a) {
  double mx, my;

  mx = 2*ra; if (mx>NX) mx=NX;
  my = 2*ra; if (my>NY) my=NY;

  for (int t=0; t<=(int)(NX*NY/(mx*my)); t++) {
    splat2D(a);
  }
}

void initan(double* buf) {
  for (int x=0; x<NX; x++) {
    for (int y=0; y<NY; y++) {
      buf[x*NY+y] = (double)x/NX;
    }
  }
}


void initam(double* buf) {
  for (int x=0; x<NX; x++) {
    for (int y=0; y<NY; y++) {
      buf[x*NY+y] = (double)y/NY;
    }
  }
}

#define TRACE(x) printf("TRACE: %s\n", #x); x

void* PongView::SmoothlifeThread(void* param) {
  PongView* self = static_cast<PongView*>(param);

  double* AA = AllocateReal();
  double* AN = AllocateReal();
  double* AM = AllocateReal();
  double* KR = AllocateReal();
  double* KD = AllocateReal();
  fftw_complex* AAF = AllocateComplex();
  fftw_complex* ANF = AllocateComplex();
  fftw_complex* AMF = AllocateComplex();
  fftw_complex* KRF = AllocateComplex();
  fftw_complex* KDF = AllocateComplex();

  fftw_plan kr_plan = fftw_plan_dft_r2c_2d(NX, NY, KR, KRF, FFTW_ESTIMATE);
  fftw_plan kd_plan = fftw_plan_dft_r2c_2d(NX, NY, KD, KDF, FFTW_ESTIMATE);
  fftw_plan aa_plan = fftw_plan_dft_r2c_2d(NX, NY, AA, AAF, FFTW_ESTIMATE);
  fftw_plan anf_plan = fftw_plan_dft_c2r_2d(NX, NY, ANF, AN, FFTW_ESTIMATE);
  fftw_plan amf_plan = fftw_plan_dft_c2r_2d(NX, NY, AMF, AM, FFTW_ESTIMATE);

  TRACE(makekernel(KR, KD));
  TRACE(fftw_execute(kr_plan));
  TRACE(fftw_execute(kd_plan));
  TRACE(inita2D(AA));

  while (!self->quit_) {
#if 0
    self->DrawBuffer(AA);
    fftw_execute(aa_plan);
    kernelmul(AAF, KRF, ANF, sqrt(NX*NY)/kflr);
    kernelmul(AAF, KDF, AMF, sqrt(NX*NY)/kfld);
    fftw_execute(anf_plan);
    fftw_execute(amf_plan);
    snm(AN, AM, AA);
#else
    initan(AN);
    initam(AM);
    snm(AN, AM, AA);
    self->DrawBuffer(AA);
#endif
  }

  fftw_destroy_plan(kr_plan);
  fftw_destroy_plan(kd_plan);
  fftw_destroy_plan(aa_plan);
  fftw_destroy_plan(anf_plan);
  fftw_destroy_plan(amf_plan);
  fftw_free(KDF);
  fftw_free(KRF);
  fftw_free(AMF);
  fftw_free(ANF);
  fftw_free(AAF);
  fftw_free(KD);
  fftw_free(KR);
  fftw_free(AM);
  fftw_free(AN);
  fftw_free(AA);
  fftw_free(AAF);
  return NULL;
}
