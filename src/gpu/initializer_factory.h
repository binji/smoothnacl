// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_INITIALIZER_FACTORY_H_
#define GPU_INITIALIZER_FACTORY_H_

#include <ppapi/cpp/size.h>
#include "initializer_factory_base.h"
#include "gpu/locked_queue.h"

class SimulationConfig;

namespace gpu {

class InitializerFactory : public InitializerFactoryBase {
 public:
  InitializerFactory(const pp::Size& size);
  virtual ~InitializerFactory();

  virtual DrawStrategyBase* CreateDrawStrategy();
  virtual SimulationBase* CreateSimulation(const SimulationConfig& config);
  virtual ViewBase* CreateView();

 private:
  pp::Size size_;
  LockedQueue* locked_queue_;
};

}  // namespace gpu

#endif  // GPU_INITIALIZER_FACTORY_H_

