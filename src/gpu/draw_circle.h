// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_DRAW_CIRCLE_H_
#define GPU_DRAW_CIRCLE_H_

#include <ppapi/cpp/size.h>
#include "gpu/shader.h"
#include "gpu/vertex_buffer.h"

namespace gpu {

class Texture;

class DrawCircle {
 public:
  explicit DrawCircle(const pp::Size& size);
  ~DrawCircle();

  void Apply(Texture& inout, float x, float y, float radius);

 private:
  pp::Size size_;
  Shader shader_;
  VertexBuffer vb_;

  DrawCircle(const DrawCircle&);  // Undefined.
  DrawCircle& operator =(const DrawCircle&);  // Undefined.
};

}  // namespace gpu

#endif  // GPU_DRAW_CIRCLE_H_

