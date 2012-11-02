// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CPU_SIMULATION_H_
#define CPU_SIMULATION_H_

#include <ppapi/cpp/size.h>
#include "cpu/kernel.h"
#include "cpu/smoother.h"
#include "fft_allocation.h"
#include "simulation_base.h"
#include "simulation_config.h"

namespace cpu {

class Simulation : public SimulationBase {
 public:
  explicit Simulation(const SimulationConfig& config);
  ~Simulation();

  virtual void SetKernel(const KernelConfig& config);
  virtual void SetSmoother(const SmootherConfig& config);
  const Kernel& kernel() const { return kernel_; }
  const Smoother& smoother() const { return smoother_; }
  const AlignedReals& buffer() const { return aa_; }

  virtual void ViewSmoother();
  virtual void Step();
  virtual void Clear(double color);
  virtual void DrawFilledCircle(double x, double y, double radius,
                                double color);
  virtual void Splat();

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

class SimulationFactory : public SimulationFactoryBase {
 public:
  virtual SimulationBase* Create(const SimulationConfig& config) {
    return new Simulation(config);
  }
};

}  // namespace cpu

#endif  // CPU_SIMULATION_H_
