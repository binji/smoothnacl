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

#ifndef KERNEL_H_
#define KERNEL_H_

#include <assert.h>
#include <ppapi/cpp/size.h>

#include "fft_allocation.h"
#include "kernel_config.h"

class Kernel {
 public:
  Kernel(const pp::Size& size, const KernelConfig& config);

  const pp::Size& size() const { return size_; }
  const KernelConfig& config() const { return config_; }
  const AlignedReals& kr() const { return kr_; }
  const AlignedReals& kd() const { return kd_; }
  const AlignedComplexes& krf() const { return krf_; }
  const AlignedComplexes& kdf() const { return kdf_; }

  void SetSize(const pp::Size& size);
  void SetConfig(const KernelConfig& config);

 private:
  void MakeKernel();

  pp::Size size_;
  KernelConfig config_;
  AlignedReals kr_;
  AlignedReals kd_;
  AlignedComplexes krf_;
  AlignedComplexes kdf_;

  Kernel(const Kernel&);  // undefined
  Kernel& operator =(const Kernel&);  // undefined
};

#endif  // KERNEL_H_
