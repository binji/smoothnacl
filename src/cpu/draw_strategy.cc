// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cpu/draw_strategy.h"
#include <algorithm>
#include "cpu/simulation.h"

namespace cpu {

DrawStrategy::DrawStrategy(LockedObject<AlignedReals>* locked_buffer)
    : locked_buffer_(locked_buffer) {
}

void DrawStrategy::Draw(ThreadDrawOptions options, SimulationBase* simulation) {
  Simulation* cpu_sim = static_cast<Simulation*>(simulation);

  switch (options) {
    default:
    case kDrawOptions_Simulation:
      CopyBuffer(cpu_sim->buffer());
      break;

    case kDrawOptions_KernelDisc:
      CopyBuffer(cpu_sim->kernel().kd());
      break;

    case kDrawOptions_KernelRing:
      CopyBuffer(cpu_sim->kernel().kr());
      break;

    case kDrawOptions_Smoother:
      cpu_sim->ViewSmoother();
      CopyBuffer(cpu_sim->buffer());
      break;
  }
}

void DrawStrategy::CopyBuffer(const AlignedReals& src) {
  ScopedLocker<AlignedReals> locker(*locked_buffer_);
  std::copy(src.begin(), src.end(), locker.object()->begin());
}

}  // namespace cpu
