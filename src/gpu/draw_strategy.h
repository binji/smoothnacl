// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DRAW_STRATEGY_H_
#define DRAW_STRATEGY_H_

#include "draw_strategy_base.h"

namespace gpu {

class DrawStrategy : public DrawStrategyBase {
 public:
  DrawStrategy();
  virtual void Draw(ThreadDrawOptions options, SimulationBase* simulation);

 private:
  DrawStrategy(const DrawStrategy&);  // Undefined.
  DrawStrategy& operator =(const DrawStrategy&);  // Undefined.
};

}  // namespace gpu

#endif  // DRAW_STRATEGY_H_

