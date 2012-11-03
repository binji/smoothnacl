// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/simulation.h"
#include <GLES2/gl2.h>
#include <math.h>

namespace gpu {

Simulation::Simulation(const SimulationConfig& config)
    : size_(config.size),
      kernel_(config.size, config.kernel_config),
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
  smoother_.Apply(an_, am_, aa_);
}

void Simulation::Step() {
  float area = size_.width() * size_.height();
  fft_.ApplyRC(aa_, aaf_);
  kernel_mul_.Apply(aaf_, kernel_.krf(), anf_, area / kernel_.kflr());
  kernel_mul_.Apply(aaf_, kernel_.kdf(), amf_, area / kernel_.kfld());
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

void Simulation::Splat() {
  // TODO(binji): implement
}

}  // namespace gpu
