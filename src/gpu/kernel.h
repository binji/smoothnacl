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

class Kernel {
 public:
  Kernel(const pp::Size& size, const KernelConfig& config);

  const pp::Size& size() const { return kernel_.size(); }
  const KernelConfig& config() const { return kernel_.config(); }
  void SetConfig(const KernelConfig& config);

  const Texture& krf() const { return krf_; }
  const Texture& kdf() const { return kdf_; }
  double kflr() const { return kernel_.kflr(); }
  double kfld() const { return kernel_.kfld(); }

 private:
  cpu::Kernel kernel_;
  Texture krf_;
  Texture kdf_;

  Kernel(const Kernel&);  // undefined
  Kernel& operator =(const Kernel&);  // undefined
};

}  // namespace gpu

#endif  // GPU_KERNEL_H_
