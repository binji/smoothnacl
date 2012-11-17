// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/kernel.h"
#include "gpu/fft.h"

namespace gpu {

Kernel::Kernel(const pp::Size& size, const KernelConfig& config, FFT* fft)
    : cpu_kernel_(size, config),
      kr_(size.width(), size.height(), FORMAT_REAL, TEXTURE_NOFRAMEBUFFER),
      kd_(size.width(), size.height(), FORMAT_REAL, TEXTURE_NOFRAMEBUFFER),
      krf_(size.width()/2 + 1, size.height(), FORMAT_COMPLEX, TEXTURE_FRAMEBUFFER),
      kdf_(size.width()/2 + 1, size.height(), FORMAT_COMPLEX, TEXTURE_FRAMEBUFFER),
      fft_(fft) {
  SetConfig(config);
}

void Kernel::SetConfig(const KernelConfig& config) {
  cpu_kernel_.SetConfig(config);
  cpu_kernel_.MakeKernel();
  kr_.Load(cpu_kernel_.kr());
  kd_.Load(cpu_kernel_.kd());
  fft_->ApplyRC(kr_, krf_);
  fft_->ApplyRC(kd_, kdf_);
}

}  // namespace gpu
