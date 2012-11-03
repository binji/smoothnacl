// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_COPYBUFFERCR_H_
#define GPU_COPYBUFFERCR_H_

#include <ppapi/cpp/size.h>
#include "gpu/shader.h"
#include "gpu/vertex_buffer.h"

namespace gpu {

class Texture;

class CopyBufferCR {
 public:
  explicit CopyBufferCR(const pp::Size& size);
  ~CopyBufferCR();

  void Apply(const Texture& in, Texture& out);

 private:
  pp::Size size_;
  Shader shader_;
  VertexBuffer vb_;

  CopyBufferCR(const CopyBufferCR&);  // Undefined.
  CopyBufferCR& operator =(const CopyBufferCR&);  // Undefined.
};

}  // namespace gpu

#endif  // GPU_COPYBUFFERCR_H_
