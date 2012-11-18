// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cpu/draw_strategy.h"
#include "cpu/simulation.h"
#include "palette.h"

namespace cpu {

DrawStrategy::DrawStrategy(LockedObject<AlignedUint32>* locked_buffer)
    : locked_buffer_(locked_buffer),
      palette_(new Palette(LabPaletteGenerator(0.3))) {
      //palette_(new Palette(WhiteOnBlackPaletteGenerator)) {
}

DrawStrategy::~DrawStrategy() {
  delete palette_;
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
  ScopedLocker<AlignedUint32> locker(*locked_buffer_);
  AlignedUint32* dst = locker.object();
  assert(src.count() == dst->count());

  for (int i = 0; i < src.count(); ++i)
    (*dst)[i] = palette_->GetColor(src[i]);
}

}  // namespace cpu
