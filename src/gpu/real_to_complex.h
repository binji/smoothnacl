// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_REAL_TO_COMPLEX_H_
#define GPU_REAL_TO_COMPLEX_H_

#include <ppapi/cpp/size.h>
#include "gpu/shader.h"
#include "gpu/vertex_buffer.h"

namespace gpu {

class Texture;

class RealToComplex {
 public:
  explicit RealToComplex(const pp::Size& size);
  ~RealToComplex();

  void Apply(const Texture& in, Texture& out);

 private:
  pp::Size size_;
  Shader shader_;
  VertexBuffer vb_;

  RealToComplex(const RealToComplex&);  // Undefined.
  RealToComplex& operator =(const RealToComplex&);  // Undefined.
};

}  // namespace gpu

#endif  // GPU_REAL_TO_COMPLEX_H_
