// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIMULATION_H_
#define SIMULATION_H_

#include "ppapi/cpp/size.h"
#include "fft_allocation.h"
#include "kernel.h"
#include "smoother.h"

struct SimulationConfig {
  pp::Size size;
  KernelConfig kernel_config;
  SmootherConfig smoother_config;
};

class Simulation {
 public:
  explicit Simulation(const SimulationConfig& config);
  ~Simulation();

  void SetKernel(const KernelConfig& config);
  void SetSmoother(const SmootherConfig& config);
  const Kernel& kernel() const { return kernel_; }
  const Smoother& smoother() const { return smoother_; }
  const AlignedReals& buffer() const { return aa_; }

  void ViewSmoother();
  void Step();
  void Clear(double color);
  void DrawFilledCircle(double x, double y, double radius, double color);
  void Splat();

 private:
  pp::Size size_;
  Kernel kernel_;
  Smoother smoother_;
  AlignedReals aa_;
  AlignedReals an_;
  AlignedReals am_;
  AlignedComplexes aaf_;
  AlignedComplexes anf_;
  AlignedComplexes amf_;
  fftw_plan aa_plan_;
  fftw_plan anf_plan_;
  fftw_plan amf_plan_;

  Simulation(const Simulation&);  // Undefined.
  Simulation& operator =(const Simulation&);  // Undefined.
};

#endif  // SIMULATION_H_
