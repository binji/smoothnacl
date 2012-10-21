#include "kernel.h"
#include <algorithm>
#include <math.h>
#include "functions.h"

namespace {

void FFT(const pp::Size& size,
         FftAllocation<double>& in,
         FftAllocation<fftw_complex>* out) {
  fftw_plan plan = fftw_plan_dft_r2c_2d(
      size.width(), size.height(), in.data(), out->data(), FFTW_ESTIMATE);
  fftw_execute(plan);
  fftw_destroy_plan(plan);
}

}  // namespace

Kernel::Kernel(const pp::Size& size, const KernelConfig& config)
    : size_(size),
      config_(config),
      dirty_(true),
      kr_(size),
      kd_(size),
      krf_(size, ReduceSizeForComplex()),
      kdf_(size, ReduceSizeForComplex()),
      kflr_(0),
      kfld_(0) {
}

void Kernel::SetConfig(const KernelConfig& config) {
  config_ = config;
  dirty_ = true;
}

const FftAllocation<double>& Kernel::GetKR() const {
  if (dirty_)
    MakeKernel();
  return kr_;
}

const FftAllocation<double>& Kernel::GetKD() const {
  if (dirty_)
    MakeKernel();
  return kd_;
}


const FftAllocation<fftw_complex>& Kernel::GetKRF() const {
  if (dirty_)
    MakeKernel();
  return krf_;
}

const FftAllocation<fftw_complex>& Kernel::GetKDF() const {
  if (dirty_)
    MakeKernel();
  return kdf_;
}

double Kernel::GetKflr() const {
  if (dirty_)
    MakeKernel();
  return kflr_;
}

double Kernel::GetKfld() const {
  if (dirty_)
    MakeKernel();
  return kfld_;
}

void Kernel::MakeKernel() const {
  double ri = config_.ra/config_.rr;
  double bb = config_.ra/config_.rb;

  // int Ra = (int)(ra+bb/2+1.0);
  int Ra = (int)(config_.ra*2);

  kflr_ = 0.0;
  kfld_ = 0.0;

  std::fill(kd_.begin(), kd_.end(), 0);
  std::fill(kr_.begin(), kr_.end(), 0);

  for (int iy = 0; iy < size_.height(); iy++) {
    int y = (iy < size_.height() / 2) ? iy : iy - size_.height();
    if (y >= -Ra && y <= Ra) {
      for (int ix=0; ix<size_.width(); ix++) {
        int x = (ix < size_.width()/2) ? ix : ix - size_.width();
        if (x >= -Ra && x <= Ra) {
          double l = sqrt(x * x + y * y);
          double m = 1 - func_linear(l, ri, bb);
          double n = func_linear(l, ri, bb) *
              (1 - func_linear(l, config_.ra, bb));
          kd_[iy * size_.width() + ix] = m;
          kr_[iy * size_.width() + ix] = n;
          kfld_ += m;
          kflr_ += n;
        }
      }
    }
  }

  FFT(size_, kd_, &kdf_);
  FFT(size_, kr_, &krf_);

  dirty_ = false;
}
