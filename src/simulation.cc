#include "simulation.h"
#include <algorithm>
#include <assert.h>
#include <fftw3.h>
#include <math.h>

namespace {

void MultiplyComplex(const AlignedComplexes& in1,
                     const AlignedComplexes& in2,
                     AlignedComplexes* out,
                     double scale) {
  int count = in1.count();
  assert(count == in2.count());
  assert(count == out->count());

  for (int i = 0; i < count; ++i) {
    const fftw_complex& inv1 = in1[i];
    const fftw_complex& inv2 = in2[i];
    fftw_complex& outv = (*out)[i];
    outv[0] = scale * (inv1[0] * inv2[0] - inv1[1] * inv2[1]);
    outv[1] = scale * (inv1[0] * inv2[1] + inv1[1] * inv2[0]);
  }
}

void Scale(AlignedReals* inout, double scale) {
  int count = inout->count();
  for (int i = 0; i < count; ++i) {
    (*inout)[i] *= scale;
  }
}

void initan(AlignedReals* buf) {
  int width = buf->size().width();
  int height = buf->size().height();
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      (*buf)[y*width+x] = (double)x/width;
    }
  }
}


void initam(AlignedReals* buf) {
  int width = buf->size().width();
  int height = buf->size().height();
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      (*buf)[y*width+x] = (double)y/height;
    }
  }
}

double RND(double x) {
  return x * (double)rand()/((double)RAND_MAX + 1);
}

}  // namespace

Simulation::Simulation(const SimulationConfig& config)
  : size_(config.size),
    kernel_(config.size, config.kernel_config),
    smoother_(config.size, config.smoother_config),
    aa_(config.size),
    an_(config.size),
    am_(config.size),
    aaf_(config.size, ReduceSizeForComplex()),
    anf_(config.size, ReduceSizeForComplex()),
    amf_(config.size, ReduceSizeForComplex()) {
  aa_plan_ = fftw_plan_dft_r2c_2d(size_.width(), size_.height(),
                                  aa_.data(), aaf_.data(), FFTW_ESTIMATE);
  anf_plan_ = fftw_plan_dft_c2r_2d(size_.width(), size_.height(),
                                   anf_.data(), an_.data(), FFTW_ESTIMATE);
  amf_plan_ = fftw_plan_dft_c2r_2d(size_.width(), size_.height(),
                                   amf_.data(), am_.data(), FFTW_ESTIMATE);

  kernel_.MakeKernel();
  smoother_.MakeLookup();
}

Simulation::~Simulation() {
  fftw_destroy_plan(amf_plan_);
  fftw_destroy_plan(anf_plan_);
  fftw_destroy_plan(aa_plan_);
}

void Simulation::SetKernel(const KernelConfig& config) {
  kernel_.SetConfig(config);
  kernel_.MakeKernel();
}

void Simulation::SetSmoother(const SmootherConfig& config) {
  smoother_.SetConfig(config);
  smoother_.MakeLookup();
}

void Simulation::ViewSmoother() {
  Clear(0);
  initan(&an_);
  initam(&am_);
  smoother_.Apply(an_, am_, &aa_);
}

void Simulation::Step() {
  int real_count = size_.width() * size_.height();
  fftw_execute(aa_plan_);
  MultiplyComplex(aaf_, kernel_.krf(), &anf_, 1.0 / kernel_.kflr());
  MultiplyComplex(aaf_, kernel_.kdf(), &amf_, 1.0 / kernel_.kfld());
  fftw_execute(anf_plan_);
  fftw_execute(amf_plan_);
  Scale(&an_, 1.0 / real_count);
  Scale(&am_, 1.0 / real_count);
  smoother_.Apply(an_, am_, &aa_);
}

void Simulation::Clear(double color) {
  std::fill(aa_.begin(), aa_.end(), color);
}

void Simulation::DrawFilledCircle(double x, double y, double radius,
                                  double color) {
  int width = aa_.size().width();
  int height = aa_.size().height();
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
        aa_[j * width + i] = color;
    }
  }
}

void Simulation::Splat() {
  double mx, my;
  int width = aa_.size().width();
  int height = aa_.size().height();

  double ring_radius = kernel_.config().ring_radius;

  mx = 2 * ring_radius; if (mx>width) mx=width;
  my = 2 * ring_radius; if (my>height) my=height;

  for (int t=0; t<=(int)(width*height/(mx*my)); t++) {
    double x = RND(width);
    double y = RND(height);
    double r = ring_radius * (RND(0.5) + 0.5);
    DrawFilledCircle(x, y, r, 1.0);
  }
}
