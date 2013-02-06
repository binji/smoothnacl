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

#ifndef INITIALIZER_FACTORY_BASE_H_
#define INITIALIZER_FACTORY_BASE_H_

class DrawStrategyBase;
class SimulationBase;
class SimulationConfig;
class ViewBase;

class InitializerFactoryBase {
 public:
  virtual ~InitializerFactoryBase() {}
  virtual DrawStrategyBase* CreateDrawStrategy() = 0;
  virtual SimulationBase* CreateSimulation(const SimulationConfig& config) = 0;
  virtual ViewBase* CreateView() = 0;
};

#endif  // INITIALIZER_FACTORY_BASE_H_
