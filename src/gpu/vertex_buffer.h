// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_VERTEX_BUFFER_H_
#define GPU_VERTEX_BUFFER_H_

#include <GLES2/gl2.h>
#include <stdlib.h>

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
  void SetAttribs(GLuint loc_pos, GLuint loc_tex0);
  void SetAttribs(GLuint loc_pos, GLuint loc_tex0, GLuint loc_tex1);
  void SetAttribs(GLuint loc_pos,
                  GLuint loc_tex0, GLuint loc_tex1, GLuint loc_tex2);
  void Draw();

 private:
  GLuint id_;
  Vertex verts_[4];  // Always a quad.
};

}  // namespace gpu

#endif  // GPU_VERTEX_BUFFER_H_
