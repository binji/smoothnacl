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
