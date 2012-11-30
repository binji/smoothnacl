// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
