// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/simulation.h"
#include <algorithm>
#include <math.h>
#include "fft_allocation.h"
#include "gpu/wrap_gl.h"

namespace gpu {

namespace {

const int kMaxSplatCircles = 5000;

double RND(double x) {
  return x * (double)rand() / ((double)RAND_MAX + 1);
}

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
      draw_circle_(config.size),
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
  draw_circle_.Apply(aa_, x, y, radius);
}

void Simulation::Splat() {
  int width = aa_.width();
  int height = aa_.height();
  double ring_radius = kernel_.config().ring_radius;
  double mx = std::min<double>(2 * ring_radius, width);
  double my = std::min<double>(2 * ring_radius, height);
  int num_circles = static_cast<int>(width * height / (mx * my));
  num_circles = std::min(num_circles, kMaxSplatCircles);

  Circles circles;
  for (int t = 0; t <= num_circles; t++) {
    Circle c;
    c.x = RND(width);
    c.y = RND(height);
    c.radius = ring_radius * (RND(0.5) + 0.5);
    circles.push_back(c);
  }

  draw_circle_.Apply(aa_, circles);
}

}  // namespace gpu
