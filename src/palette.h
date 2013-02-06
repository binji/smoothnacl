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

#ifndef PALETTE_H_
#define PALETTE_H_

#include <stdint.h>
#include <stdlib.h>
#include <vector>

struct ColorStop {
  ColorStop(uint32_t color, double pos);

  uint32_t color;
  double pos;
};
typedef std::vector<ColorStop> ColorStops;

struct PaletteConfig {
  PaletteConfig();

  std::vector<ColorStop> stops;
  bool repeating;
};

class Palette {
 public:
  explicit Palette(const PaletteConfig& config);
  uint32_t GetColor(double value) const;

  void SetConfig(const PaletteConfig& config);

 private:
  static const size_t kColorMapSize = 512;
  uint32_t value_color_map_[kColorMapSize];
};

#endif  // PALETTE_H_
