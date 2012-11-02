// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/kernel.h"

namespace gpu {

Kernel::Kernel(const pp::Size& size, const KernelConfig& config)
    : kernel_(size, config),
      krf_(size.width(), size.height(), FORMAT_COMPLEX, TEXTURE_NOFRAMEBUFFER),
      kdf_(size.width(), size.height(), FORMAT_COMPLEX, TEXTURE_NOFRAMEBUFFER) {
}

void Kernel::SetConfig(const KernelConfig& config) {
  kernel_.SetConfig(config);
  kernel_.MakeKernel();
}

}  // namespace gpu
