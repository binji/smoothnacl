// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_VERTEX_BUFFER_H_
#define GPU_VERTEX_BUFFER_H_

#include <stdlib.h>
#include "gpu/wrap_gl.h"

namespace gpu {

struct Vertex {
  GLfloat tex[3][2];  // 3 pairs of UVs.
  GLfloat pos[3];  // XYZ.
};

class VertexBuffer {
 public:
  VertexBuffer();
  ~VertexBuffer();

  void Clear();
  void SetSize(float width, float height);
  void SetTex(size_t index, float left, float top, float right, float bottom);
  void LoadData();
  void SetAttribs(Location loc_pos, Location loc_tex0);
  void SetAttribs(Location loc_pos, Location loc_tex0, Location loc_tex1);
  void SetAttribs(Location loc_pos,
                  Location loc_tex0, Location loc_tex1, Location loc_tex2);
  void Draw();

 private:
  ID id_;
  Vertex verts_[4];  // Always a quad.

  VertexBuffer(const VertexBuffer&);  // undefined
  VertexBuffer& operator =(const VertexBuffer&);  // undefined
};

}  // namespace gpu

#endif  // GPU_VERTEX_BUFFER_H_
