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

#ifndef CPU_SMOOTHER_H_
#define CPU_SMOOTHER_H_

#include "fft_allocation.h"
#include "smoother_config.h"

namespace cpu {

class Smoother {
 public:
  Smoother(const pp::Size& size, const SmootherConfig& config);

  const pp::Size& size() const { return size_; }
  const SmootherConfig& config() const { return config_; }
  void SetConfig(const SmootherConfig& config);
  void Apply(const AlignedReals& buf1,
             const AlignedReals& buf2,
             AlignedReals* out) const;
  void MakeLookup();

 private:
  double CalculateValue(double n, double m) const;
  double Lookup(double n, double m) const;
  void Apply_Discrete(const double* an, const double* am, double* na) const;
  void Apply_Smooth1(const double* an, const double* am, double* na) const;
  void Apply_Smooth2(const double* an, const double* am, double* na) const;
  void Apply_Smooth3(const double* an, const double* am, double* na) const;
  void Apply_Smooth4(const double* an, const double* am, double* na) const;

  pp::Size size_;
  SmootherConfig config_;
  bool dirty_;
  AlignedReals lookup_;

  Smoother(const Smoother&);
  Smoother& operator =(const Smoother&);
};

}  // namespace cpu

#endif  // CPU_SMOOTHER_H_
