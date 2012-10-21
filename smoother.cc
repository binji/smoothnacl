#include "smoother.h"
#include "functions.h"

namespace {

const int kLookupSize = 1024;

double my_hard(double x, double a, double) {
  return func_hard(x, a);
}

typedef double (*SigmoidFunc)(double, double, double);
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

double mix(double x, double y, double m) {
  return x + m * (y - x);
}

double sigmoid_ab(SigmoidFunc f, double sn, double x, double a, double b) {
  return (*f)(x, a, sn)*(1.0 - (*f)(x, b, sn));
}

double sigmoid_mix(SigmoidFunc f, double sm, double x, double y, double m) {
  return x + (*f)(m, 0.5, sm) * (y - x);
}

double clamp01(double x) {
  return x > 1.0 ? 1.0 : x < 0.0 ? 0.0 : x;
}

}  // namespace

Smoother::Smoother(const pp::Size& size, const SmootherConfig& config)
    : size_(size),
      config_(config),
      dirty_(true),
      lookup_(pp::Size(kLookupSize, kLookupSize)) {
}

void Smoother::SetConfig(const SmootherConfig& config) {
  config_ = config;
  dirty_ = true;
}

void Smoother::Apply(const AlignedReals& buf1, const AlignedReals& buf2,
                     AlignedReals* out) const {
  assert(!dirty_);
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
      double n = static_cast<double>(i)/kLookupSize;
      double m = static_cast<double>(j)/kLookupSize;
      lookup_[i * kLookupSize + j] = clamp01(CalculateValue(n, m));
    }
  }

  dirty_ = false;
}

double Smoother::CalculateValue(double n, double m) const {
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

double Smoother::Lookup(double n, double m) const {
  return lookup_[static_cast<int>(n * kLookupSize) * kLookupSize +
    static_cast<int>(m * kLookupSize)];
}

void Smoother::Apply_Discrete(const double* an, const double* am,
                              double* na) const {
  int count = size_.width() * size_.height();
  for (int i = 0; i < count; ++i) {
    na[i] = Lookup(an[i], am[i]);
  }
}

void Smoother::Apply_Smooth1(const double* an, const double* am,
                             double* na) const {
  int count = size_.width() * size_.height();
  for (int i = 0; i < count; ++i) {
    double f = Lookup(an[i], am[i]);
    na[i] = clamp01(na[i] + config_.timestep.dt * (2 * f - 1));
  }
}

void Smoother::Apply_Smooth2(const double* an, const double* am,
                             double* na) const {
  int count = size_.width() * size_.height();
  for (int i = 0; i < count; ++i) {
    double f = Lookup(an[i], am[i]);
    na[i] = clamp01(na[i] + config_.timestep.dt * (f - na[i]));
  }
}

void Smoother::Apply_Smooth3(const double* an, const double* am,
                             double* na) const {
  int count = size_.width() * size_.height();
  for (int i = 0; i < count; ++i) {
    double f = Lookup(an[i], am[i]);
    na[i] = clamp01(am[i] + config_.timestep.dt * (2 * f - 1));
  }
}

void Smoother::Apply_Smooth4(const double* an, const double* am,
                             double* na) const {
  int count = size_.width() * size_.height();
  for (int i = 0; i < count; ++i) {
    double f = Lookup(an[i], am[i]);
    na[i] = clamp01(am[i] + config_.timestep.dt * (f - am[i]));
  }
}
