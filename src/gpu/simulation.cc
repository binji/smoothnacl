// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/simulation.h"
#include <GLES2/gl2.h>

namespace gpu {
namespace {

}  // namespace

Simulation::Simulation(const SimulationConfig& config)
    : size_(config.size),
      kernel_(config.size, config.kernel_config),
      kernel_mul_(config.size) {
}

Simulation::~Simulation() {
}

void Simulation::SetKernel(const KernelConfig& config) {
  kernel_.SetConfig(config);
}

void Simulation::SetSmoother(const SmootherConfig& config) {
}

void Simulation::ViewSmoother() {
}

void Simulation::Step() {
}

void Simulation::Clear(double color) {
}

void Simulation::DrawFilledCircle(double x, double y, double radius,
                                  double color) {
}

void Simulation::Splat() {
}

}  // namespace gpu
