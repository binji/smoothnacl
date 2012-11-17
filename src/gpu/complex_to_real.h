// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_COMPLEX_TO_REAL_H_
#define GPU_COMPLEX_TO_REAL_H_

#include <ppapi/cpp/size.h>
#include "gpu/shader.h"
#include "gpu/vertex_buffer.h"

namespace gpu {

class Texture;

class ComplexToReal {
 public:
  explicit ComplexToReal(const pp::Size& size);
  ~ComplexToReal();

  void Apply(const Texture& in, Texture& out);

 private:
  pp::Size size_;
  Shader shader_;
  VertexBuffer vb_;

  ComplexToReal(const ComplexToReal&);  // Undefined.
  ComplexToReal& operator =(const ComplexToReal&);  // Undefined.
};

}  // namespace gpu

#endif  // GPU_COMPLEX_TO_REAL_H_
