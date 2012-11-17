// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

