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

#ifndef SIMULATION_H_
#define SIMULATION_H_

#include <ppapi/cpp/size.h>

#include "kernel.h"
#include "smoother.h"

#include "fftw.h"
#include "fft_allocation.h"
#include "simulation_config.h"

class Simulation {
 public:
  explicit Simulation(const SimulationConfig& config);
  ~Simulation();

  const pp::Size& size() const { return size_; }
  void SetKernel(const KernelConfig& config);
  void SetSmoother(const SmootherConfig& config);
  const Kernel& kernel() const { return kernel_; }
  const Smoother& smoother() const { return smoother_; }
  const AlignedReals& buffer() const { return aa_; }

  void Step();
  void Clear(real color);
  void DrawFilledCircle(real x, real y, real radius, real color);
  void Splat();

 private:
  void DrawFilledCircleNoWrap(real x, real y, real radius, real color);

  pp::Size size_;
  Kernel kernel_;
  Smoother smoother_;
  AlignedReals aa_;
  AlignedReals an_;
  AlignedReals am_;
  AlignedComplexes aaf_;
  AlignedComplexes tempf_;
  fftw_plan aa_plan_;
  fftw_plan an_plan_;
  fftw_plan am_plan_;

  Simulation(const Simulation&);  // Undefined.
  Simulation& operator =(const Simulation&);  // Undefined.
};

#endif  // SIMULATION_H_
