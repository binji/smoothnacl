#ifndef PALETTE_H_
#define PALETTE_H_

#include <stdint.h>
#include <stdlib.h>

class PaletteGenerator;

class Palette {
 public:
  // Takes ownership of |generator|.
  explicit Palette(PaletteGenerator* generator);
  ~Palette();
  uint32_t GetColor(double value) const;

 private:
  void MakeLookupTable();

  static const size_t kColorMapSize = 512;
  PaletteGenerator* generator_;
  uint32_t value_color_map_[kColorMapSize];
};

class PaletteGenerator {
 public:
  virtual uint32_t GetColor(double value) const = 0;
};

class WhiteOnBlackPaletteGenerator : public PaletteGenerator {
 public:
  virtual uint32_t GetColor(double value) const;
};

class BlackOnWhitePaletteGenerator : public PaletteGenerator {
 public:
  virtual uint32_t GetColor(double value) const;
};

class LabPaletteGenerator : public PaletteGenerator {
 public:
  explicit LabPaletteGenerator(double c);
  virtual uint32_t GetColor(double value) const;

 private:
  double c_;
};

#endif  // PALETTE_H_
