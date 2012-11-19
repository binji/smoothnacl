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
