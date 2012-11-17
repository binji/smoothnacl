// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_SHADER_H_
#define GPU_SHADER_H_

#include <map>
#include <string>
#include "gpu/wrap_gl.h"

namespace gpu {

class Texture;

class Shader {
 public:
  Shader();
  ~Shader();

  void Init(const char* frag_shader, const char* vertex_shader);

  Location GetAttribLocation(const char* name);
  Location GetUniformLocation(const char* name);
  void Use();
  void Uniform1f(const char* name, GLfloat value);
  void Uniform1i(const char* name, GLint value);
  void UniformTexture(const char* name, int index, const Texture& texture);
  void UniformMatrixOrtho(const char* name, float l, float r, float b, float t,
                          float near, float far);

 private:
  PassID CompileShader(GLenum type, const char* data);

  ID id_;
  ID vert_id_;
  ID frag_id_;
  typedef std::map<std::string, Location> NameLocationMap;
  NameLocationMap attribs_;
  NameLocationMap uniforms_;

  Shader(const Shader&);  // undefined
  Shader& operator =(const Shader&);  // undefined
};

}  // namespace gpu

#endif  // GPU_SHADER_H_
