// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DRAW_STRATEGY_BASE_H_
#define DRAW_STRATEGY_BASE_H_

#include "simulation_thread_options.h"

struct PaletteConfig;
struct ScreenshotConfig;
class SimulationBase;

class DrawStrategyBase {
 public:
  virtual ~DrawStrategyBase() {}
  virtual void Draw(SimulationThreadDrawOptions options,
                    SimulationBase* simulation) = 0;
  virtual void SetPalette(const PaletteConfig& config) {}
  virtual void PostScreenshot(const ScreenshotConfig& config) {}
};

#endif  // DRAW_STRATEGY_BASE_H_
