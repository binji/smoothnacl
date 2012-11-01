// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
