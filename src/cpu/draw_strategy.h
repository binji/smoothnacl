// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CPU_DRAW_STRATEGY_H_
#define CPU_DRAW_STRATEGY_H_

#include "draw_strategy_base.h"
#include "fft_allocation.h"
#include "locked_object.h"
#include "palette.h"

namespace cpu {

class DrawStrategy : public DrawStrategyBase {
 public:
  explicit DrawStrategy(LockedObject<AlignedUint32>* locked_buffer);
  virtual void Draw(SimulationThreadDrawOptions options,
                    SimulationBase* simulation);
  virtual void SetPalette(const PaletteConfig& config);

 private:
  void CopyBuffer(const AlignedReals& src);

  LockedObject<AlignedUint32>* locked_buffer_;  // Weak.
  Palette palette_;

  DrawStrategy(const DrawStrategy&);  // Undefined.
  DrawStrategy& operator =(const DrawStrategy&);  // Undefined.
};

}  // namespace

#endif  // CPU_DRAW_STRATEGY_H_

