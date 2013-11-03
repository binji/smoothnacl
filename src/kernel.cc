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

#include "fftw.h"
#include "functions.h"

namespace {

void FFT(const pp::Size& size, AlignedReals& in, AlignedComplexes* out) {
  fftw_plan plan = fftw_plan_dft_r2c_2d(
      size.width(), size.height(), in.data(), out->data(), FFTW_ESTIMATE);
  fftw_execute(plan);
  fftw_destroy_plan(plan);
}

void Scale(AlignedComplexes* c, real scale) {
  for (fftw_complex* i = c->begin(), *e = c->end(); i != e; ++i) {
    (*i)[0] *= scale;
    (*i)[1] *= scale;
  }
}

}  // namespace

Kernel::Kernel(const pp::Size& size, const KernelConfig& config)
    : size_(size),
      config_(config),
      kr_(size),
      kd_(size),
      krf_(size, ReduceSizeForComplex()),
      kdf_(size, ReduceSizeForComplex()) {
}

void Kernel::SetSize(const pp::Size& size) {
  size_ = size;
  AlignedReals(size).swap(kr_);
  AlignedReals(size).swap(kd_);
  AlignedComplexes(size, ReduceSizeForComplex()).swap(krf_);
  AlignedComplexes(size, ReduceSizeForComplex()).swap(kdf_);
  MakeKernel();
}

void Kernel::SetConfig(const KernelConfig& config) {
  config_ = config;
  MakeKernel();
}

void Kernel::MakeKernel() {
  real ri = config_.disc_radius;
  real bb = config_.blend_radius;

  int Ra = (int)(config_.ring_radius*2);

  real kflr = 0.0;
  real kfld = 0.0;

  std::fill(kd_.begin(), kd_.end(), 0);
  std::fill(kr_.begin(), kr_.end(), 0);

  for (int iy = 0; iy < size_.height(); iy++) {
    int y = (iy < size_.height() / 2) ? iy : iy - size_.height();
    if (y >= -Ra && y <= Ra) {
      for (int ix=0; ix<size_.width(); ix++) {
        int x = (ix < size_.width()/2) ? ix : ix - size_.width();
        if (x >= -Ra && x <= Ra) {
          real l = sqrt(x * x + y * y);
          real m = 1 - func_linear(l, ri, bb);
          real n = func_linear(l, ri, bb) *
              (1 - func_linear(l, config_.ring_radius, bb));
          kd_[iy * size_.width() + ix] = m;
          kr_[iy * size_.width() + ix] = n;
          kfld += m;
          kflr += n;
        }
      }
    }
  }

  FFT(size_, kd_, &kdf_);
  Scale(&kdf_, 1.0 / kfld);

  FFT(size_, kr_, &krf_);
  Scale(&krf_, 1.0 / kflr);
}
