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

#ifndef CPU_DRAW_STRATEGY_H_
#define CPU_DRAW_STRATEGY_H_

#include "draw_strategy_base.h"
#include "fft_allocation.h"
#include "locked_object.h"
#include "palette.h"
#include "screenshot_config.h"

namespace pp {
class Instance;
}  // namespace pp

namespace cpu {

class DrawStrategy : public DrawStrategyBase {
 public:
  DrawStrategy(pp::Instance* instance,
               LockedObject<AlignedUint32>* locked_buffer);
  virtual void Draw(SimulationThreadDrawOptions options,
                    SimulationBase* simulation);
  virtual void SetPalette(const PaletteConfig& config);
  virtual void PostScreenshot(const ScreenshotConfig& config);

 private:
  void CopyBuffer(const AlignedReals& src);

  pp::Instance* instance_;  // Weak.
  LockedObject<AlignedUint32>* locked_buffer_;  // Weak.
  Palette palette_;

  DrawStrategy(const DrawStrategy&);  // Undefined.
  DrawStrategy& operator =(const DrawStrategy&);  // Undefined.
};

}  // namespace

#endif  // CPU_DRAW_STRATEGY_H_

