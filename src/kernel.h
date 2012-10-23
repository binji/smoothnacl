#ifndef KERNEL_H_
#define KERNEL_H_

#include <assert.h>
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
  const AlignedReals& kr() const { assert(!dirty_); return kr_; }
  const AlignedReals& kd() const { assert(!dirty_); return kd_; }
  const AlignedComplexes& krf() const { assert(!dirty_); return krf_; }
  const AlignedComplexes& kdf() const { assert(!dirty_); return kdf_; }
  double kflr() const { assert(!dirty_); return kflr_; }
  double kfld() const { assert(!dirty_); return kfld_; }

  void MakeKernel();

 private:
  pp::Size size_;
  KernelConfig config_;
  bool dirty_;
  AlignedReals kr_;
  AlignedReals kd_;
  AlignedComplexes krf_;
  AlignedComplexes kdf_;
  double kflr_;
  double kfld_;

  Kernel(const Kernel&);  // undefined
  Kernel& operator =(const Kernel&);  // undefined
};

#endif  // KERNEL_H_
