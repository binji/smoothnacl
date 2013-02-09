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
  virtual AlignedReals* GetBuffer();
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
