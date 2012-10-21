#ifndef SIMULATION_H_
#define SIMULATION_H_

#include "ppapi/cpp/size.h"
#include "fft_allocation.h"
#include "kernel.h"
#include "smoother.h"

class Simulation {
 public:
  Simulation(const pp::Size& size,
             const KernelConfig& kernel_config,
             const SmootherConfig& smoother_config);
  ~Simulation();

  void SetKernel(const KernelConfig& config);
  void SetSmoother(const SmootherConfig& config);
  const Kernel& kernel() const { return kernel_; }
  const Smoother& smoother() const { return smoother_; }
  const FftAllocation<double>& buffer() const { return aa_; }

  void Step();
  void Clear(double color);
  void DrawFilledCircle(double x, double y, double radius, double color);
  void inita2D(double radius);

 private:
  pp::Size size_;
  Kernel kernel_;
  Smoother smoother_;
  FftAllocation<double> aa_;
  FftAllocation<double> an_;
  FftAllocation<double> am_;
  FftAllocation<fftw_complex> aaf_;
  FftAllocation<fftw_complex> anf_;
  FftAllocation<fftw_complex> amf_;
  fftw_plan aa_plan_;
  fftw_plan anf_plan_;
  fftw_plan amf_plan_;
};

#endif  // SIMULATION_H_
