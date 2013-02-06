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

#ifndef CPU_INITIALIZER_FACTORY_H_
#define CPU_INITIALIZER_FACTORY_H_

#include <ppapi/cpp/size.h>
#include "fft_allocation.h"
#include "initializer_factory_base.h"
#include "locked_object.h"

class SimulationConfig;

namespace pp {
class Instance;
}  // namespace pp

namespace cpu {

class InitializerFactory : public InitializerFactoryBase {
 public:
  InitializerFactory(pp::Instance* instance, const pp::Size& size);
  virtual ~InitializerFactory();

  virtual DrawStrategyBase* CreateDrawStrategy();
  virtual SimulationBase* CreateSimulation(const SimulationConfig& config);
  virtual ViewBase* CreateView();

 private:
  pp::Instance* instance_;  // Weak.
  LockedObject<AlignedUint32>* locked_buffer_;
};

}  // namespace cpu

#endif  // CPU_INITIALIZER_FACTORY_H_
