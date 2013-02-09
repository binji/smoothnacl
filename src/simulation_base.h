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

#ifndef SIMULATION_BASE_H_
#define SIMULATION_BASE_H_

#include "fft_allocation.h"

class KernelConfig;
class SimulationConfig;
class SmootherConfig;

class SimulationBase {
 public:
  virtual ~SimulationBase() {}
  virtual void SetKernel(const KernelConfig& config) = 0;
  virtual void SetSmoother(const SmootherConfig& config) = 0;
  virtual AlignedReals GetBuffer() = 0;

  virtual void ViewSmoother() = 0;
  virtual void Step() = 0;
  virtual void Clear(double color) = 0;
  virtual void DrawFilledCircle(double x, double y,
                                double radius,
                                double color) = 0;
  virtual void Splat() = 0;
};

#endif  // SIMULATION_BASE_H_
