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
  return new gpu::View(size_, locked_queue_);
}

}  // namespace gpu
