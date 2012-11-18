#include "palette.h"
#include <algorithm>
#include <math.h>

namespace {

/* Copyright (c) David Dalrymple 2011 */

double finv(double t) {
  return (t>(6.0/29.0))?(t*t*t):(3*(6.0/29.0)*(6.0/29.0)*(t-4.0/29.0));
}

/* Convert from L*a*b* doubles to XYZ doubles
 * Formulas drawn from http://en.wikipedia.org/wiki/Lab_color_space
 */
void lab2xyz(double* x, double* y, double* z, double l, double a, double b) {
  double sl = (l+0.16)/1.16;
  double ill[3] = {0.9643,1.00,0.8251}; //D50
  *y = ill[1] * finv(sl);
  *x = ill[0] * finv(sl + (a/5.0));
  *z = ill[2] * finv(sl - (b/2.0));
}

double correct(double cl) {
  double a = 0.055;
  return (cl<=0.0031308)?(12.92*cl):((1+a)*pow(cl,1/2.4)-a);
}

/* Convert from XYZ doubles to sRGB bytes
 * Formulas drawn from http://en.wikipedia.org/wiki/Srgb
 */
void xyz2rgb(unsigned char* r, unsigned char* g, unsigned char* b, double x, double y, double z) {
  double rl =  3.2406*x - 1.5372*y - 0.4986*z;
  double gl = -0.9689*x + 1.8758*y + 0.0415*z;
  double bl =  0.0557*x - 0.2040*y + 1.0570*z;
  int clip = (rl < 0.0 || rl > 1.0 || gl < 0.0 || gl > 1.0 || bl < 0.0 || bl > 1.0);
  if(clip) {
    rl = (rl<0.0)?0.0:((rl>1.0)?1.0:rl);
    gl = (gl<0.0)?0.0:((gl>1.0)?1.0:gl);
    bl = (bl<0.0)?0.0:((bl>1.0)?1.0:bl);
  }
  //Uncomment the below to detect clipping by making clipped zones red.
  if(clip) {rl=1.0;gl=bl=0.0;}
  *r = (unsigned char)(255.0*correct(rl));
  *g = (unsigned char)(255.0*correct(gl));
  *b = (unsigned char)(255.0*correct(bl));
}

/* Convert from LAB doubles to sRGB bytes
 * (just composing the above transforms)
 */
void lab2rgb(uint8_t* R, uint8_t* G, uint8_t* B, double l, double a, double b) {
  double x,y,z;
  lab2xyz(&x,&y,&z,l,a,b);
  xyz2rgb(R,G,B,x,y,z);
}

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
  LabPaletteGenerator(double a, double b);
  virtual uint32_t GetColor(double value) const;

 private:
  double a_;
  double b_;
};

uint32_t WhiteOnBlackPaletteGenerator::GetColor(double value) const {
  uint8_t v = 255 * value;
  uint32_t color = 0xff000000 | (v<<16) | (v<<8) | v;
  return color;
}

uint32_t BlackOnWhitePaletteGenerator::GetColor(double value) const {
  uint8_t v = 255 * (1 - value);
  uint32_t color = 0xff000000 | (v<<16) | (v<<8) | v;
  return color;
}

LabPaletteGenerator::LabPaletteGenerator(double a, double b)
    : a_(a),
      b_(b) {
}

uint32_t LabPaletteGenerator::GetColor(double value) const {
#if 0
  // also known as "two pi" to the unenlightened
  const double TAU = 6.283185307179586476925287;

  /* Convert from a qualitative parameter c and a quantitative parameter l to
   * a 24-bit pixel These formulas were invented by me to obtain maximum
   * contrast without going out of gamut if the parameters are in the range
   * 0-1
   */
  double L = value*0.61+0.09; //L of L*a*b*
  double angle = TAU/6.0-c_*TAU;
  double r = value*0.311+0.125; //~chroma
  double a = sin(angle)*r;
  double b = cos(angle)*r;
#else
  double L = value;
  double a = 2 * a_ - 1;
  double b = 2 * b_ - 1;
#endif
  uint8_t r8;
  uint8_t g8;
  uint8_t b8;
  lab2rgb(&r8,&g8,&b8,L,a,b);
  uint32_t color = 0xff000000 | (r8<<16) | (g8<<8) | b8;
  return color;
}

template <size_t size>
void MakeLookupTable(const PaletteGenerator& generator,
                     uint32_t (*color_map)[size]) {
  for (size_t i = 0; i < size; ++i) {
    (*color_map)[i] = generator.GetColor(static_cast<double>(i) / size);
  }
}

}  // namespace

PaletteConfig::PaletteConfig()
    : type(PALETTE_WHITE_ON_BLACK),
      a(0),
      b(0) {
}

Palette::Palette(const PaletteConfig& config) {
  SetConfig(config);
}

uint32_t Palette::GetColor(double value) const {
  size_t index = static_cast<size_t>(value * kColorMapSize);
  index = std::max<size_t>(0, std::min(kColorMapSize - 1, index));
  return value_color_map_[index];
}

void Palette::SetConfig(const PaletteConfig& config) {
  switch (config.type) {
    default:
    case PALETTE_WHITE_ON_BLACK:
      MakeLookupTable(WhiteOnBlackPaletteGenerator(), &value_color_map_);
      break;
    case PALETTE_BLACK_ON_WHITE:
      MakeLookupTable(BlackOnWhitePaletteGenerator(), &value_color_map_);
      break;
    case PALETTE_LAB:
      MakeLookupTable(LabPaletteGenerator(config.a, config.b),
                      &value_color_map_);
      break;
  }
}
