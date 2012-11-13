// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/initializer_factory.h"
#include "gpu/draw_strategy.h"
#include "gpu/simulation.h"
#include "gpu/view.h"
#include "simulation_config.h"

namespace gpu {
namespace {

const int MAX_QUEUED_FRAMES = 3;

}  // namespace

InitializerFactory::InitializerFactory(const pp::Size& size)
    : size_(size) {
  locked_queue_ = new LockedQueue(MAX_QUEUED_FRAMES);
}

InitializerFactory::~InitializerFactory() {
  delete locked_queue_;
}

DrawStrategyBase* InitializerFactory::CreateDrawStrategy() {
  return new gpu::DrawStrategy(size_, locked_queue_);
}

SimulationBase* InitializerFactory::CreateSimulation(
    const SimulationConfig& config) {
  return new gpu::Simulation(config);
}

ViewBase* InitializerFactory::CreateView() {
  return new gpu::View(locked_queue_);
}

}  // namespace gpu
