// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/vertex_buffer.h"
#include <stddef.h>
#include <string.h>

namespace gpu {

VertexBuffer::VertexBuffer() {
  glGenBuffers(1, &id_);
  Clear();
}

VertexBuffer::~VertexBuffer() {
}


void VertexBuffer::Clear() {
  memset(&verts_[0], 0, sizeof(verts_));
}

void VertexBuffer::SetSize(float width, float height) {
  verts_[0].pos[0] = 0;
  verts_[0].pos[1] = 0;
  verts_[1].pos[0] = width;
  verts_[1].pos[1] = 0;
  verts_[2].pos[0] = 0;
  verts_[2].pos[1] = height;
  verts_[3].pos[0] = width;
  verts_[3].pos[1] = height;
}

void VertexBuffer::SetTex(size_t index, float left, float top, float right,
                          float bottom) {
  verts_[0].tex[index][0] = left;
  verts_[0].tex[index][1] = top;
  verts_[1].tex[index][0] = right;
  verts_[1].tex[index][1] = top;
  verts_[2].tex[index][0] = left;
  verts_[2].tex[index][1] = bottom;
  verts_[3].tex[index][0] = right;
  verts_[3].tex[index][1] = bottom;
}

void VertexBuffer::LoadData() {
  glBindBuffer(GL_ARRAY_BUFFER, id_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts_), &verts_[0], GL_STATIC_DRAW);
}

void VertexBuffer::SetAttribs(Location loc_pos) {
  glBindBuffer(GL_ARRAY_BUFFER, id_);
  glVertexAttribPointer(loc_pos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void*>(offsetof(Vertex, pos)));
  glEnableVertexAttribArray(loc_pos);
}

void VertexBuffer::SetAttribs(Location loc_pos, Location loc_tex0) {
  SetAttribs(loc_pos);
  glVertexAttribPointer(loc_tex0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void*>(offsetof(Vertex, tex[0])));
  glEnableVertexAttribArray(loc_tex0);
}

void VertexBuffer::SetAttribs(Location loc_pos, Location loc_tex0,
                              Location loc_tex1) {
  SetAttribs(loc_pos, loc_tex0);
  glVertexAttribPointer(loc_tex1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void*>(offsetof(Vertex, tex[1])));
  glEnableVertexAttribArray(loc_tex1);
}

void VertexBuffer::SetAttribs(Location loc_pos, Location loc_tex0,
                              Location loc_tex1, Location loc_tex2) {
  SetAttribs(loc_pos, loc_tex0, loc_tex1);
  glVertexAttribPointer(loc_tex2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void*>(offsetof(Vertex, tex[2])));
  glEnableVertexAttribArray(loc_tex2);
}

void VertexBuffer::Draw() {
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

}  // namespace gpu
