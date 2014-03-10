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

#include "palette.h"
#include <algorithm>
#include <math.h>

#include <ppapi/c/ppb_image_data.h>
#include <ppapi/cpp/image_data.h>

namespace {

const uint32_t kBlack = 0xff000000;

real NonnegativeFmod(real x, real y) {
  return fmod(fmod(x, y) + y, y);
}

void Uint32ToRGB(uint32_t c, uint8_t* r, uint8_t* g, uint8_t* b) {
  *r = (c >> 16) & 0xff;
  *g = (c >> 8) & 0xff;
  *b = (c >> 0) & 0xff;
}

uint32_t RGBToUint32(uint8_t r, uint8_t g, uint8_t b) {
  static PP_ImageDataFormat format = pp::ImageData::GetNativeImageDataFormat();
  if (format == PP_IMAGEDATAFORMAT_BGRA_PREMUL) {
    return 0xff000000 | (r << 16) | (g << 8) | b;
  } else if (format == PP_IMAGEDATAFORMAT_RGBA_PREMUL) {
    return 0xff000000 | (b << 16) | (g << 8) | r;
  } else {
    assert(0);
    return 0xff000000;
  }
}

template <typename T>
T Mix(T t0, T t1, real x) {
  return static_cast<T>(t0 * (1 - x) + t1 * x);
}

uint32_t MixColor(uint32_t c0, uint32_t c1, real x) {
  uint8_t r0, g0, b0;
  uint8_t r1, g1, b1;
  Uint32ToRGB(c0, &r0, &g0, &b0);
  Uint32ToRGB(c1, &r1, &g1, &b1);
  return RGBToUint32(Mix(r0, r1, x), Mix(g0, g1, x), Mix(b0, b1, x));
}

class PaletteGenerator {
 public:
  virtual uint32_t GetColor(real value) const = 0;
};

class GradientPaletteGenerator : public PaletteGenerator {
 public:
  GradientPaletteGenerator(const ColorStops& stops, bool repeating);
  virtual uint32_t GetColor(real value) const;

 private:
  real GetMinStopPos() const;
  real GetMaxStopPos() const;

  ColorStops stops_;
  bool repeating_;
  real min_pos_;
  real max_pos_;
};

GradientPaletteGenerator::GradientPaletteGenerator(const ColorStops& stops,
                                                   bool repeating)
    : stops_(stops),
      repeating_(repeating),
      min_pos_(GetMinStopPos()),
      max_pos_(GetMaxStopPos()) {
}

uint32_t GradientPaletteGenerator::GetColor(real value) const {
  if (stops_.empty())
    return kBlack;

  if (repeating_)
    value = NonnegativeFmod(value - min_pos_, max_pos_ - min_pos_) + min_pos_;

  if (value < min_pos_)
    return stops_[0].color;

  real range_min = 0;
  for (size_t i = 0; i < stops_.size() - 1; ++i) {
    range_min = std::max(range_min, stops_[i].pos);
    real range_max = stops_[i + 1].pos;
    if (range_min >= range_max)
      continue;

    if (value < range_min || value > range_max)
      continue;

    real mix_fraction = (value - range_min) / (range_max - range_min);
    return MixColor(stops_[i].color, stops_[i + 1].color, mix_fraction);
  }

  return stops_[stops_.size() - 1].color;
}

real GradientPaletteGenerator::GetMinStopPos() const {
  return stops_.empty() ? 0 : stops_[0].pos;
}

real GradientPaletteGenerator::GetMaxStopPos() const {
  if (stops_.empty())
    return 0;

  real maxpos = stops_[0].pos;
  for (size_t i = 1; i < stops_.size(); ++i) {
    maxpos = std::max(stops_[i].pos, maxpos);
  }
  return maxpos;
}

template <size_t size>
void MakeLookupTable(const PaletteGenerator& generator,
                     uint32_t (*color_map)[size]) {
  for (size_t i = 0; i < size; ++i) {
    (*color_map)[i] = generator.GetColor(static_cast<real>(i) / size);
  }
}

}  // namespace

ColorStop::ColorStop(uint32_t color, real pos)
    : color(color),
      pos(pos) {
}

PaletteConfig::PaletteConfig()
    : repeating(false) {
}

Palette::Palette(const PaletteConfig& config) {
  SetConfig(config);
}

uint32_t Palette::GetColor(real value) const {
  size_t index = static_cast<size_t>(value * kColorMapSize);
  index = std::max<size_t>(0, std::min(kColorMapSize - 1, index));
  return value_color_map_[index];
}

void Palette::SetConfig(const PaletteConfig& config) {
  MakeLookupTable(GradientPaletteGenerator(config.stops, config.repeating),
                  &value_color_map_);
}
