// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIMULATION_BASE_H_
#define SIMULATION_BASE_H_

class KernelConfig;
class SimulationConfig;
class SmootherConfig;

class SimulationBase {
 public:
  virtual ~SimulationBase() {}
  virtual void SetKernel(const KernelConfig& config) = 0;
  virtual void SetSmoother(const SmootherConfig& config) = 0;

  virtual void ViewSmoother() = 0;
  virtual void Step() = 0;
  virtual void Clear(double color) = 0;
  virtual void DrawFilledCircle(double x, double y,
                                double radius,
                                double color) = 0;
  virtual void Splat() = 0;
};

class SimulationFactoryBase {
 public:
  virtual ~SimulationFactoryBase() {}
  virtual SimulationBase* Create(const SimulationConfig& config) = 0;
};

#endif  // SIMULATION_BASE_H_
