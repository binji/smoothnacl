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

#include "smoother.h"
#include "functions.h"

namespace {

const int kLookupSize = 256;

real my_hard(real x, real a, real) {
  return func_hard(x, a);
}

typedef real (*SigmoidFunc)(real, real, real);
SigmoidFunc GetFunc(Sigmoid s) {
  switch (s) {
    case SIGMOID_HARD: return &my_hard;
    case SIGMOID_LINEAR: return &func_linear;
    case SIGMOID_HERMITE: return &func_hermite;
    case SIGMOID_SIN: return &func_sin;
    default:
    case SIGMOID_SMOOTH: return &func_smooth;
  }
}

real mix(real x, real y, real m) {
  return x + m * (y - x);
}

real sigmoid_ab(SigmoidFunc f, real sn, real x, real a, real b) {
  return (*f)(x, a, sn)*(1.0 - (*f)(x, b, sn));
}

real sigmoid_mix(SigmoidFunc f, real sm, real x, real y, real m) {
  return x + (*f)(m, 0.5, sm) * (y - x);
}

real clamp01(real x) {
  return x > 1.0 ? 1.0 : x < 0.0 ? 0.0 : x;
}

}  // namespace

Smoother::Smoother(const pp::Size& size, const SmootherConfig& config)
    : size_(size),
      config_(config),
      lookup_(pp::Size(kLookupSize, kLookupSize)) {
}

void Smoother::SetSize(const pp::Size& size) {
  size_ = size;
}

void Smoother::SetConfig(const SmootherConfig& config) {
  config_ = config;
  MakeLookup();
}

void Smoother::Apply(const AlignedReals& buf1, const AlignedReals& buf2,
                     AlignedReals* out) const {
  switch (config_.timestep.type) {
    default:
    case TIMESTEP_DISCRETE:
      Apply_Discrete(buf1.data(), buf2.data(), out->data());
      break;
    case TIMESTEP_SMOOTH1:
      Apply_Smooth1(buf1.data(), buf2.data(), out->data());
      break;
    case TIMESTEP_SMOOTH2:
      Apply_Smooth2(buf1.data(), buf2.data(), out->data());
      break;
    case TIMESTEP_SMOOTH3:
      Apply_Smooth3(buf1.data(), buf2.data(), out->data());
      break;
    case TIMESTEP_SMOOTH4:
      Apply_Smooth4(buf1.data(), buf2.data(), out->data());
      break;
  }
}

void Smoother::MakeLookup() {
  for (int i = 0; i < kLookupSize; ++i) {
    for (int j = 0; j < kLookupSize; ++j) {
      real n = static_cast<real>(i)/kLookupSize;
      real m = static_cast<real>(j)/kLookupSize;
      lookup_[i * kLookupSize + j] = clamp01(CalculateValue(n, m));
    }
  }
}

real Smoother::CalculateValue(real n, real m) const {
  SigmoidFunc ab_func = GetFunc(config_.sigmoid);
  SigmoidFunc mix_func = GetFunc(config_.mix);

  switch (config_.mode) {
    case SIGMOID_MODE_1:
      return mix(
          sigmoid_ab(ab_func, config_.sn, n, config_.b1, config_.b2),
          sigmoid_ab(ab_func, config_.sn, n, config_.d1, config_.d2),
          m);
    case SIGMOID_MODE_2:
      return sigmoid_mix(
          mix_func,
          config_.sm,
          sigmoid_ab(ab_func, config_.sn, n, config_.b1, config_.b2),
          sigmoid_ab(ab_func, config_.sn, n, config_.d1, config_.d2),
          m);
    case SIGMOID_MODE_3:
      return sigmoid_ab(
          ab_func,
          config_.sn,
          n,
          mix(config_.b1, config_.d1, m),
          mix(config_.b2, config_.d2, m));
    default:
    case SIGMOID_MODE_4:
      return sigmoid_ab(
          ab_func,
          config_.sn,
          n,
          sigmoid_mix(mix_func, config_.sm, config_.b1, config_.d1, m),
          sigmoid_mix(mix_func, config_.sm, config_.b2, config_.d2, m));
  }
}

real Smoother::Lookup(real n, real m) const {
  return lookup_[static_cast<int>(n * kLookupSize) * kLookupSize +
                 static_cast<int>(m * kLookupSize)];
}

void Smoother::Apply_Discrete(const real* an, const real* am, real* na) const {
  int count = size_.width() * size_.height();
  real scale = 1.0 / count;
  for (int i = 0; i < count; ++i) {
    real ani = an[i] * scale;
    real ami = am[i] * scale;
    na[i] = Lookup(ani, ami);
  }
}

void Smoother::Apply_Smooth1(const real* an, const real* am, real* na) const {
  int count = size_.width() * size_.height();
  real scale = 1.0 / count;
  for (int i = 0; i < count; ++i) {
    real ani = an[i] * scale;
    real ami = am[i] * scale;
    real f = Lookup(ani, ami);
    na[i] = clamp01(na[i] + config_.timestep.dt * (2 * f - 1));
  }
}

void Smoother::Apply_Smooth2(const real* an, const real* am, real* na) const {
  int count = size_.width() * size_.height();
  real scale = 1.0 / count;
  for (int i = 0; i < count; ++i) {
    real ani = an[i] * scale;
    real ami = am[i] * scale;
    real f = Lookup(ani, ami);
    na[i] = clamp01(na[i] + config_.timestep.dt * (f - na[i]));
  }
}

void Smoother::Apply_Smooth3(const real* an, const real* am, real* na) const {
  int count = size_.width() * size_.height();
  real scale = 1.0 / count;
  for (int i = 0; i < count; ++i) {
    real ani = an[i] * scale;
    real ami = am[i] * scale;
    real f = Lookup(ani, ami);
    na[i] = clamp01(ami + config_.timestep.dt * (2 * f - 1));
  }
}

void Smoother::Apply_Smooth4(const real* an, const real* am, real* na) const {
  int count = size_.width() * size_.height();
  real scale = 1.0 / count;
  for (int i = 0; i < count; ++i) {
    real ani = an[i] * scale;
    real ami = am[i] * scale;
    real f = Lookup(ani, ami);
    na[i] = clamp01(ami + config_.timestep.dt * (f - ami));
  }
}
