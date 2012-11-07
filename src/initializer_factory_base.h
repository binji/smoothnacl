// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
