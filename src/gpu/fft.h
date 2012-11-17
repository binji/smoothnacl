// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_FFT_H_
#define GPU_FFT_H_

#include <ppapi/cpp/size.h>
#include "gpu/complex_to_real.h"
#include "gpu/fft_stage.h"
#include "gpu/real_to_complex.h"
#include "gpu/texture.h"

namespace gpu {

class FFT {
 public:
  explicit FFT(const pp::Size& size);
  ~FFT();

  void ApplyRC(const Texture& in, Texture& out);
  void ApplyCR(const Texture& in, Texture& out);

 private:
  pp::Size size_;
  ComplexToReal complex_to_real_;
  RealToComplex real_to_complex_;
  FFTStage fft_stage_;
  Texture tex_;
  Texture tex2_;

  FFT(const FFT&);  // undefined
  FFT& operator =(const FFT&);  // undefined
};

}  // namespace gpu

#endif  // GPU_FFT_H_
