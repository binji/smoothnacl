// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/kernel.h"

namespace gpu {

Kernel::Kernel(const pp::Size& size, const KernelConfig& config)
    : kernel_(size, config),
      kr_(size.width(), size.height(), FORMAT_REAL, TEXTURE_NOFRAMEBUFFER),
      kd_(size.width(), size.height(), FORMAT_REAL, TEXTURE_NOFRAMEBUFFER),
      krf_(size.width()/2 + 1, size.height(), FORMAT_COMPLEX, TEXTURE_NOFRAMEBUFFER),
      kdf_(size.width()/2 + 1, size.height(), FORMAT_COMPLEX, TEXTURE_NOFRAMEBUFFER) {
}

void Kernel::SetConfig(const KernelConfig& config) {
  kernel_.SetConfig(config);
  kernel_.MakeKernel();
  kr_.Load(kernel_.kr());
  kd_.Load(kernel_.kd());
  krf_.Load(kernel_.krf());
  kdf_.Load(kernel_.kdf());
}

}  // namespace gpu
