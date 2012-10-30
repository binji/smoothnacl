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
  //if(clip) {rl=1.0;gl=bl=0.0;}
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

}  // namespace

Palette::Palette(PaletteGenerator* generator)
    : generator_(generator) {
  MakeLookupTable();
}

Palette::~Palette() {
  delete generator_;
}

uint32_t Palette::GetColor(double value) const {
  size_t index = static_cast<size_t>(value * kColorMapSize);
  index = std::max<size_t>(0, std::min(kColorMapSize - 1, index));
  return value_color_map_[index];
}

void Palette::MakeLookupTable() {
  for (size_t i = 0; i < kColorMapSize; ++i) {
    value_color_map_[i] =
        generator_->GetColor(static_cast<double>(i) / kColorMapSize);
  }
}

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

LabPaletteGenerator::LabPaletteGenerator(double c)
    : c_(c) {
}

uint32_t LabPaletteGenerator::GetColor(double value) const {
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
  uint8_t r8;
  uint8_t g8;
  uint8_t b8;
  lab2rgb(&r8,&g8,&b8,L,a,b);
  uint32_t color = 0xff000000 | (b8<<16) | (g8<<8) | r8;
  return color;
}
