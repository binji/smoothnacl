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

#include <fftw3.h>

#include "fft_allocation.h"
#include "kernel_config.h"

class Kernel {
 public:
  Kernel(const pp::Size& size, const KernelConfig& config);

  const pp::Size& size() const { return size_; }
  const KernelConfig& config() const { return config_; }
  void SetConfig(const KernelConfig& config);
  const AlignedReals& kr() const { assert(!dirty_); return kr_; }
  const AlignedReals& kd() const { assert(!dirty_); return kd_; }
  const AlignedComplexes& krf() const { assert(!dirty_); return krf_; }
  const AlignedComplexes& kdf() const { assert(!dirty_); return kdf_; }
  double kflr() const { assert(!dirty_); return kflr_; }
  double kfld() const { assert(!dirty_); return kfld_; }

  void MakeKernel();

 private:
  pp::Size size_;
  KernelConfig config_;
  bool dirty_;
  AlignedReals kr_;
  AlignedReals kd_;
  AlignedComplexes krf_;
  AlignedComplexes kdf_;
  double kflr_;
  double kfld_;

  Kernel(const Kernel&);  // undefined
  Kernel& operator =(const Kernel&);  // undefined
};

#endif  // KERNEL_H_
