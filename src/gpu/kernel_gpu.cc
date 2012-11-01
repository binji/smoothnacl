// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "kernel_gpu.h"

KernelGPU::KernelGPU(const pp::Size& size, const KernelConfig& config)
    : kernel_(size, config),
      KRF_(0),
      KDF_(0) {
}

KernelGPU::~KernelGPU() {
}

void KernelGPU::SetConfig(const KernelConfig& config) {
  kernel_.SetConfig(config);
  kernel_.MakeKernel();
}
