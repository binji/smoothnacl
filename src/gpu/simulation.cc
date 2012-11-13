// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/simulation.h"
#include <math.h>
#include "fft_allocation.h"
#include "gpu/wrap_gl.h"

namespace gpu {

namespace {

void initan(Texture* buf) {
  int width = buf->width();
  int height = buf->height();
  AlignedFloats floats(pp::Size(width, height));
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      floats[y*width+x] = (double)x/width;
    }
  }
  buf->Load(floats);
}

void initam(Texture* buf) {
  int width = buf->width();
  int height = buf->height();
  AlignedFloats floats(pp::Size(width, height));
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      floats[y*width+x] = (double)y/height;
    }
  }
  buf->Load(floats);
}

}  // namespace

Simulation::Simulation(const SimulationConfig& config)
    : size_(config.size),
      kernel_(config.size, config.kernel_config, &fft_),
      kernel_mul_(config.size),
      smoother_(config.size, config.smoother_config),
      fft_(config.size),
      aa_(config.size.width(), config.size.height(), FORMAT_REAL, TEXTURE_FRAMEBUFFER),
      an_(config.size.width(), config.size.height(), FORMAT_REAL, TEXTURE_FRAMEBUFFER),
      am_(config.size.width(), config.size.height(), FORMAT_REAL, TEXTURE_FRAMEBUFFER),
      aaf_(config.size.width()/2 + 1, config.size.height(), FORMAT_COMPLEX, TEXTURE_FRAMEBUFFER),
      anf_(config.size.width()/2 + 1, config.size.height(), FORMAT_COMPLEX, TEXTURE_FRAMEBUFFER),
      amf_(config.size.width()/2 + 1, config.size.height(), FORMAT_COMPLEX, TEXTURE_FRAMEBUFFER) {
}

Simulation::~Simulation() {
}

void Simulation::SetKernel(const KernelConfig& config) {
  kernel_.SetConfig(config);
}

void Simulation::SetSmoother(const SmootherConfig& config) {
  smoother_.SetConfig(config);
}

void Simulation::ViewSmoother() {
  Clear(0);
  initan(&an_);
  initam(&am_);
  smoother_.Apply(an_, am_, aa_);
}

void Simulation::Step() {
  float sqrt_area = sqrt(size_.width() * size_.height());
  fft_.ApplyRC(aa_, aaf_);
  kernel_mul_.Apply(aaf_, kernel_.krf(), anf_, sqrt_area / kernel_.kflr());
  kernel_mul_.Apply(aaf_, kernel_.kdf(), amf_, sqrt_area / kernel_.kfld());
  fft_.ApplyCR(anf_, an_);
  fft_.ApplyCR(amf_, am_);
  smoother_.Apply(an_, am_, aa_);
}

void Simulation::Clear(double color) {
  aa_.BindFramebuffer();
  glClearColor(color, color, color, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Simulation::DrawFilledCircle(double x, double y, double radius,
                                  double color) {
  // TODO(binji): implement
}

static double RND(double x) {
  return x * (double)rand()/((double)RAND_MAX + 1);
}

void Simulation::Splat() {
  // TODO(binji): implement
  double mx, my;
  int width = aa_.width();
  int height = aa_.height();
  AlignedFloats fs(pp::Size(width, height));

  double ring_radius = kernel_.config().ring_radius;

  mx = 2 * ring_radius; if (mx>width) mx=width;
  my = 2 * ring_radius; if (my>height) my=height;

  for (int t=0; t<=(int)(width*height/(mx*my)); t++) {
    double x = RND(width);
    double y = RND(height);
    double radius = ring_radius * (RND(0.5) + 0.5);

    int width = aa_.width();
    int height = aa_.height();
    int left = std::max(0, static_cast<int>(x - radius));
    int right = std::min(width, static_cast<int>(x + radius + 1));
    int top = std::max(0, static_cast<int>(y - radius));
    int bottom = std::min(height, static_cast<int>(y + radius + 1));

    for (int j = top; j < bottom; ++j) {
      for (int i = left; i < right; ++i) {
        double dx = x - i;
        double dy = y - j;
        double length = sqrt(dx * dx + dy * dy);
        if (length < radius)
          fs[j * width + i] = 1.0;
      }
    }
  }

  aa_.Load(fs);
}

}  // namespace gpu
