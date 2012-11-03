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

class CopyBufferRC {
 public:
  explicit CopyBufferRC(const pp::Size& size);
  ~CopyBufferRC();

  void Apply(const Texture& in, Texture& out);

 private:
  pp::Size size_;
  Shader shader_;
  VertexBuffer vb_;

  CopyBufferRC(const CopyBufferRC&);  // Undefined.
  CopyBufferRC& operator =(const CopyBufferRC&);  // Undefined.
};

}  // namespace gpu

#endif  // GPU_COPYBUFFERCR_H_
