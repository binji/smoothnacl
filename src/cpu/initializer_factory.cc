// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cpu/initializer_factory.h"
#include "cpu/draw_strategy.h"
#include "cpu/simulation.h"
#include "cpu/view.h"
#include "simulation_config.h"

namespace cpu {

InitializerFactory::InitializerFactory(pp::Instance* instance,
                                       const pp::Size& size)
    : instance_(instance) {
  AlignedUint32* buffer = new AlignedUint32(size);
  locked_buffer_ = new LockedObject<AlignedUint32>(buffer);
}

InitializerFactory::~InitializerFactory() {
  delete locked_buffer_;
}

DrawStrategyBase* InitializerFactory::CreateDrawStrategy() {
  return new cpu::DrawStrategy(instance_, locked_buffer_);
}

SimulationBase* InitializerFactory::CreateSimulation(
    const SimulationConfig& config) {
  return new cpu::Simulation(config);
}

ViewBase* InitializerFactory::CreateView() {
  return new cpu::View(locked_buffer_);
}

}  // namespace cpu
