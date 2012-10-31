// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SMOOTHER_H_
#define SMOOTHER_H_

#include "fft_allocation.h"

enum Timestep {
  TIMESTEP_DISCRETE,
  TIMESTEP_SMOOTH1,
  TIMESTEP_SMOOTH2,
  TIMESTEP_SMOOTH3,
  TIMESTEP_SMOOTH4
};

struct TimestepConfig {
  Timestep type;
  double dt;
};

enum Sigmoid {
  SIGMOID_HARD,
  SIGMOID_LINEAR,
  SIGMOID_HERMITE,
  SIGMOID_SIN,
  SIGMOID_SMOOTH
};

enum SigmoidMode {
  // mix(sigmoid_ab(n, b1, b2), sigmoid_ab(n, d1, d2), m)
  SIGMOID_MODE_1,
  // sigmoid_mix(sigmoid_ab(n, b1, b2), sigmoid_ab(n, d1, d2), m)
  SIGMOID_MODE_2,
  // sigmoid_ab(n, mix(b1, d1, m), mix(b1, b2, m))
  SIGMOID_MODE_3,
  // sigmoid_ab(n, sigmoid_mix(b1, d1, m), sigmoid_mix(b1, b2, m))
  SIGMOID_MODE_4
};

struct SmootherConfig {
  TimestepConfig timestep;
  double b1;
  double d1;
  double b2;
  double d2;
  SigmoidMode mode;
  Sigmoid sigmoid;
  Sigmoid mix;
  double sn;
  double sm;
};

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

#endif  // SMOOTHER_H_
