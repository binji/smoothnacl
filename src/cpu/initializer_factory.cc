// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cpu/initializer_factory.h"
#include "cpu/draw_strategy.h"
#include "cpu/simulation.h"
#include "cpu/view.h"
#include "simulation_config.h"

namespace cpu {

InitializerFactory::InitializerFactory(const pp::Size& size) {
  AlignedReals* buffer = new AlignedReals(size);
  locked_buffer_ = new LockedObject<AlignedReals>(buffer);
}

InitializerFactory::~InitializerFactory() {
  delete locked_buffer_;
}

DrawStrategyBase* InitializerFactory::CreateDrawStrategy() {
  return new cpu::DrawStrategy(locked_buffer_);
}

SimulationBase* InitializerFactory::CreateSimulation(
    const SimulationConfig& config) {
  return new cpu::Simulation(config);
}

ViewBase* InitializerFactory::CreateView() {
  return new cpu::View(locked_buffer_);
}

}  // namespace cpu
