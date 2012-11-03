// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_SIMULATION_H_
#define GPU_SIMULATION_H_

#include <ppapi/cpp/size.h>
#include "gpu/kernel.h"
#include "gpu/kernel_mul.h"
#include "gpu/shader.h"
#include "gpu/smoother.h"
#include "gpu/vertex_buffer.h"
#include "simulation_base.h"
#include "simulation_config.h"

namespace gpu {

class Simulation : public SimulationBase {
 public:
  explicit Simulation(const SimulationConfig& config);
  ~Simulation();

  virtual void SetKernel(const KernelConfig& config);
  virtual void SetSmoother(const SmootherConfig& config);
  const Kernel& kernel() const { return kernel_; }

  virtual void ViewSmoother();
  virtual void Step();
  virtual void Clear(double color);
  virtual void DrawFilledCircle(double x, double y, double radius,
                                double color);
  virtual void Splat();

 private:
  pp::Size size_;
  Kernel kernel_;
  KernelMul kernel_mul_;
  Smoother smoother_;

  Simulation(const Simulation&);  // Undefined.
  Simulation& operator =(const Simulation&);  // Undefined.
};

class SimulationFactory : public SimulationFactoryBase {
 public:
  virtual SimulationBase* Create(const SimulationConfig& config) {
    return new Simulation(config);
  }
};

}  // namespace gpu

#endif  // GPU_SIMULATION_H_
