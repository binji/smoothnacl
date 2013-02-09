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

#include "cpu/draw_strategy.h"
#include "cpu/simulation.h"

namespace cpu {

DrawStrategy::DrawStrategy(LockedObject<AlignedUint32>* locked_buffer)
    : locked_buffer_(locked_buffer),
      palette_(PaletteConfig()) {
}

void DrawStrategy::Draw(SimulationThreadDrawOptions options,
                        SimulationBase* simulation) {
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

    case kDrawOptions_Palette: {
      ScopedLocker<AlignedUint32> locker(*locked_buffer_);
      AlignedUint32* dst = locker.object();

      int width = dst->size().width();
      int height = dst->size().height();
      for (int y = 0; y < height; ++y)
      for (int x = 0; x < width; ++x)
        (*dst)[y * width + x] = palette_.GetColor(x / (double)width);
      break;
    }
  }
}

void DrawStrategy::SetPalette(const PaletteConfig& config) {
  palette_.SetConfig(config);
}

AlignedUint32* DrawStrategy::GetDrawBuffer() {
  AlignedUint32* buffer = locked_buffer_->Lock();
  AlignedUint32* copy = new AlignedUint32(*buffer);
  locked_buffer_->Unlock();

  return copy;
}

void DrawStrategy::CopyBuffer(const AlignedReals& src) {
  ScopedLocker<AlignedUint32> locker(*locked_buffer_);
  AlignedUint32* dst = locker.object();
  assert(src.count() == dst->count());

  for (int i = 0; i < src.count(); ++i)
    (*dst)[i] = palette_.GetColor(src[i]);
}

}  // namespace cpu
