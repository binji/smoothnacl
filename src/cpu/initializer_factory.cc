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
