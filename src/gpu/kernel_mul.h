// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_KERNEL_MUL_H_
#define GPU_KERNEL_MUL_H_

#include <ppapi/cpp/size.h>
#include "gpu/shader.h"
#include "gpu/vertex_buffer.h"

namespace gpu {

class Texture;

class KernelMul {
 public:
  explicit KernelMul(const pp::Size& size);
  ~KernelMul();

  void Apply(const Texture& in0, const Texture& in1, Texture& out,
             double scale);

 private:
  pp::Size size_;
  Shader shader_;
  VertexBuffer vb_;

  KernelMul(const KernelMul&);  // Undefined.
  KernelMul& operator =(const KernelMul&);  // Undefined.
};

}  // namespace gpu

#endif  // GPU_KERNEL_MUL_H_
