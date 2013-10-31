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

#include "kernel.h"

#include <algorithm>
#include <math.h>

#include "functions.h"

namespace {

void FFT(const pp::Size& size, AlignedReals& in, AlignedComplexes* out) {
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

void Kernel::MakeKernel() {
  double ri = config_.disc_radius;
  double bb = config_.blend_radius;

  int Ra = (int)(config_.ring_radius*2);

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
              (1 - func_linear(l, config_.ring_radius, bb));
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
