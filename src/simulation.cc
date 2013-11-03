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

#include "simulation.h"

#include <algorithm>
#include <assert.h>
#include <math.h>

#include "timer.h"
#include "wisdom.h"

namespace {

void MultiplyComplex(const AlignedComplexes& in1,
                     const AlignedComplexes& in2,
                     AlignedComplexes* out) {
  int count = in1.count();
  assert(count == in2.count());
  assert(count == out->count());

  for (int i = 0; i < count; ++i) {
    const fftw_complex& inv1 = in1[i];
    const fftw_complex& inv2 = in2[i];
    fftw_complex& outv = (*out)[i];
    outv[0] = inv1[0] * inv2[0] - inv1[1] * inv2[1];
    outv[1] = inv1[0] * inv2[1] + inv1[1] * inv2[0];
  }
}

real RND(real x) {
  return x * (real)rand()/((real)RAND_MAX + 1);
}

}  // namespace

#define CHECK(x) \
  do { \
    result = (int)x; \
    if (!result) { \
      printf("%s failed.\n", #x); \
      exit(1); \
    } \
  } while(0)

#ifdef USE_WISDOM
#define PLAN_FLAGS FFTW_WISDOM_ONLY
#else
#define PLAN_FLAGS FFTW_ESTIMATE
#endif

Simulation::Simulation(const SimulationConfig& config)
  : size_(config.size),
    kernel_(config.size, config.kernel_config),
    smoother_(config.size, config.smoother_config),
    aa_(config.size),
    an_(config.size),
    am_(config.size),
    aaf_(config.size, ReduceSizeForComplex()),
    tempf_(config.size, ReduceSizeForComplex()) {
  int result;
#ifdef USE_THREADS
  CHECK(fftw_init_threads());
  fftw_plan_with_nthreads(4);
#endif
  // I haven't made any ARM wisdom yet; it requires building sel_ldr_arm, and
  // running fftw-wisdom under QEMU.
#if defined(USE_WISDOM) && !defined(__arm__)
  CHECK(fftw_import_wisdom_from_string(kWisdom512));
#endif
  aa_plan_ = fftw_plan_dft_r2c_2d(size_.width(), size_.height(),
                                  aa_.data(), aaf_.data(), PLAN_FLAGS);
  CHECK(aa_plan_);
  an_plan_ = fftw_plan_dft_c2r_2d(size_.width(), size_.height(),
                                  tempf_.data(), an_.data(), PLAN_FLAGS);
  CHECK(an_plan_);
  am_plan_ = fftw_plan_dft_c2r_2d(size_.width(), size_.height(),
                                  tempf_.data(), am_.data(), PLAN_FLAGS);
  CHECK(am_plan_);

  kernel_.MakeKernel();
  smoother_.MakeLookup();
}

Simulation::~Simulation() {
  fftw_destroy_plan(am_plan_);
  fftw_destroy_plan(an_plan_);
  fftw_destroy_plan(aa_plan_);
#ifdef USE_THREADS
  fftw_cleanup_threads();
#endif
}

void Simulation::SetKernel(const KernelConfig& config) {
  kernel_.SetConfig(config);
  kernel_.MakeKernel();
}

void Simulation::SetSmoother(const SmootherConfig& config) {
  smoother_.SetConfig(config);
  smoother_.MakeLookup();
}

//#undef TIME
//#define TIME(x) x

void Simulation::Step() {
  TIME(fftw_execute(aa_plan_));
  TIME(MultiplyComplex(aaf_, kernel_.krf(), &tempf_));
  TIME(fftw_execute(an_plan_));
  TIME(MultiplyComplex(aaf_, kernel_.kdf(), &tempf_));
  TIME(fftw_execute(am_plan_));
  TIME(smoother_.Apply(an_, am_, &aa_));
}

void Simulation::Clear(real color) {
  std::fill(aa_.begin(), aa_.end(), color);
}

void Simulation::DrawFilledCircle(real x, real y, real radius, real color) {
  int width = aa_.size().width();
  int height = aa_.size().height();
  int ix = static_cast<int>(x) % width;
  int iy = static_cast<int>(y) % height;

  const int kOverlapLeft = 1;
  const int kOverlapRight = 2;
  const int kOverlapTop = 1;
  const int kOverlapBottom = 2;
  int overlap_x = 0;
  int overlap_y = 0;
  if (ix - radius < 0) overlap_x |= kOverlapLeft;
  if (ix + radius + 1 > width) overlap_x |= kOverlapRight;
  if (iy - radius < 0) overlap_y |= kOverlapTop;
  if (iy + radius + 1 > height) overlap_y |= kOverlapBottom;

  for (int ox = 0; ox < 3; ++ox) {
    int nx = ix;
    if (ox & kOverlapLeft) nx += width;
    if (ox & kOverlapRight) nx -= width;

    for (int oy = 0; oy < 3; ++oy) {
      int ny = iy;
      if (oy & kOverlapTop) ny += height;
      if (oy & kOverlapBottom) ny -= height;

      if ((!ox || (overlap_x & ox)) && (!oy || (overlap_y & oy)))
        DrawFilledCircleNoWrap(nx, ny, radius, color);
    }
  }
}

void Simulation::DrawFilledCircleNoWrap(real x, real y, real radius,
                                        real color) {
  int width = aa_.size().width();
  int height = aa_.size().height();
  int left = std::max(0, static_cast<int>(x - radius));
  int right = std::min(width, static_cast<int>(x + radius + 1));
  int top = std::max(0, static_cast<int>(y - radius));
  int bottom = std::min(height, static_cast<int>(y + radius + 1));

  for (int j = top; j < bottom; ++j) {
    for (int i = left; i < right; ++i) {
      real dx = x - i;
      real dy = y - j;
      real length = dx * dx + dy * dy;
      if (length < radius * radius)
        aa_[j * width + i] = color;
    }
  }
}

void Simulation::Splat() {
  real mx, my;
  int width = aa_.size().width();
  int height = aa_.size().height();

  real ring_radius = kernel_.config().ring_radius;

  mx = 2 * ring_radius; if (mx>width) mx=width;
  my = 2 * ring_radius; if (my>height) my=height;

  for (int t=0; t<=(int)(width*height/(mx*my)); t++) {
    real x = RND(width);
    real y = RND(height);
    real r = ring_radius * (RND(0.5) + 0.5);
    DrawFilledCircle(x, y, r, 1.0);
  }
}
