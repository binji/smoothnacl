#ifndef KERNEL_H_
#define KERNEL_H_

#include <fftw3.h>
#include "ppapi/cpp/size.h"
#include "fft_allocation.h"

struct KernelConfig {
  // Radius of the neighborhood of a "cell"; this is the area that can
  // potentially affect this cell.
  double ra;
  // Inverse factor to scale the inner radius of the cell.
  // For example, if ra=30, rr=3, the inner radius is 10.
  double rr;  
  // The anti-aliasing radius, expressed as an inverse factor of ra.
  // For example, if ra=30, rr=3, rb=15, the anti-aliasing radius is 2.
  //   The region [0, 9) is inside the cell.
  //   The region [9, 11) is inside the anti-aliasing region.
  //   The region [11, 30) is inside the neighborhood.
  double rb;
};

class Kernel {
 public:
  Kernel(const pp::Size& size, const KernelConfig& config);

  const pp::Size& size() const { return size_; }
  const KernelConfig& config() const { return config_; }
  void SetConfig(const KernelConfig& config);
  const FftAllocation<double>& GetKR() const;
  const FftAllocation<double>& GetKD() const;
  const FftAllocation<fftw_complex>& GetKRF() const;
  const FftAllocation<fftw_complex>& GetKDF() const;
  double GetKflr() const;
  double GetKfld() const;

 private:
  void MakeKernel() const;

  pp::Size size_;
  KernelConfig config_;
  mutable bool dirty_;
  mutable FftAllocation<double> kr_;
  mutable FftAllocation<double> kd_;
  mutable FftAllocation<fftw_complex> krf_;
  mutable FftAllocation<fftw_complex> kdf_;
  mutable double kflr_;
  mutable double kfld_;

  Kernel(const Kernel&);  // undefined
  Kernel& operator =(const Kernel&);  // undefined
};

#endif  // KERNEL_H_
