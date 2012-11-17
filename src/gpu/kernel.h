// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_KERNEL_H_
#define GPU_KERNEL_H_

#include "cpu/kernel.h"
#include "gpu/texture.h"
#include "gpu/wrap_gl.h"

struct KernelConfig;

namespace gpu {

class FFT;

class Kernel {
 public:
  Kernel(const pp::Size& size, const KernelConfig& config, FFT* fft);

  const pp::Size& size() const { return cpu_kernel_.size(); }
  const KernelConfig& config() const { return cpu_kernel_.config(); }
  void SetConfig(const KernelConfig& config);

  const Texture& kr() const { return kr_; }
  const Texture& kd() const { return kd_; }
  const Texture& krf() const { return krf_; }
  const Texture& kdf() const { return kdf_; }
  double kflr() const { return cpu_kernel_.kflr(); }
  double kfld() const { return cpu_kernel_.kfld(); }

 private:
  cpu::Kernel cpu_kernel_;
  Texture kr_;
  Texture kd_;
  Texture krf_;
  Texture kdf_;
  FFT* fft_;  // Weak.

  Kernel(const Kernel&);  // undefined
  Kernel& operator =(const Kernel&);  // undefined
};

}  // namespace gpu

#endif  // GPU_KERNEL_H_
