// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_SIMULATION_H_
#define GPU_SIMULATION_H_

#include <ppapi/cpp/size.h>
#include "gpu/draw_circle.h"
#include "gpu/fft.h"
#include "gpu/kernel.h"
#include "gpu/kernel_mul.h"
#include "gpu/shader.h"
#include "gpu/smoother.h"
#include "gpu/texture.h"
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
  const Texture& aa() const { return aa_; }
  const Texture& aaf() const { return aaf_; }

  virtual void ViewSmoother();
  virtual void Step();
  virtual void Clear(double color);
  virtual void DrawFilledCircle(double x, double y, double radius,
                                double color);
  virtual void Splat();

 private:
  pp::Size size_;
  FFT fft_;
  Kernel kernel_;
  KernelMul kernel_mul_;
  Smoother smoother_;
  DrawCircle draw_circle_;
  Texture aa_;
  Texture an_;
  Texture am_;
  Texture aaf_;
  Texture anf_;
  Texture amf_;

  Simulation(const Simulation&);  // Undefined.
  Simulation& operator =(const Simulation&);  // Undefined.
};

}  // namespace gpu

#endif  // GPU_SIMULATION_H_
