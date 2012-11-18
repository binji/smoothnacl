#ifndef PALETTE_H_
#define PALETTE_H_

#include <stdint.h>
#include <stdlib.h>

class PaletteGenerator;

enum PaletteType {
  PALETTE_WHITE_ON_BLACK,
  PALETTE_BLACK_ON_WHITE,
  PALETTE_LAB,
};

struct PaletteConfig {
  PaletteConfig();

  PaletteType type;
  double a;
  double b;
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
