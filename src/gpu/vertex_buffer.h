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
  void SetAttribs(Location loc_pos);
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
