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
