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

#ifndef GPU_FFT_STAGE_H_
#define GPU_FFT_STAGE_H_

#include <ppapi/cpp/size.h>
#include <stdint.h>
#include "gpu/shader.h"
#include "gpu/vertex_buffer.h"

namespace gpu {

enum FFTSign {
  // These values are used to index the Texture2 array below.
  FFT_SIGN_NEGATIVE = 0,
  FFT_SIGN_POSITIVE = 1,
};

class Texture;
typedef float float4[4];

class FFTStage {
 public:
  explicit FFTStage(const pp::Size& size);
  ~FFTStage();

  uint32_t log2w() const { return log2w_; }
  uint32_t log2h() const { return log2h_; }

  void ApplyX(int index, FFTSign sign, const Texture& in, Texture& out);
  void ApplyY(int index, FFTSign sign, const Texture& in, Texture& out);

 private:
  void MakeAllPlanX();
  void MakePlanX(float sign, int index, float4* buffer);
  void MakeAllPlanY();
  void MakePlanY(float sign, int index, float4* buffer);

  typedef Texture* Texture2[2];
  pp::Size size_;
  Shader shader_;
  VertexBuffer vb_;
  VertexBuffer vb2_;
  Texture2* planx_;
  Texture2* plany_;
  uint32_t log2w_;
  uint32_t log2h_;
};

}  // namespace gpu

#endif  // GPU_FFT_STAGE_H_

