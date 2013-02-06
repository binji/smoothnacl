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
