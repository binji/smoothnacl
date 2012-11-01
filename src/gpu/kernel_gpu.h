// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef KERNEL_H_
#define KERNEL_H_

#include <GLES2/gl2.h>
#include "kernel.h"
#include "texture.h"

class KernelGPU {
 public:
  KernelGPU(const pp::Size& size, const KernelConfig& config);
  ~KernelGPU();

  const pp::Size& size() const { return kernel_.size(); }
  const KernelConfig& config() const { return kernel_.config(); }
  void SetConfig(const KernelConfig& config);
  double kflr() const { return kernel_.kflr(); }
  double kfld() const { return kernel_.kfld(); }

 private:
  Kernel kernel_;
  Texture krf_;
  Texture kdf_;

  KernelGPU(const KernelGPU&);  // undefined
  KernelGPU& operator =(const KernelGPU&);  // undefined
};

#endif  // KERNEL_H_
