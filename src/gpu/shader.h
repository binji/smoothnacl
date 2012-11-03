// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_SHADER_H_
#define GPU_SHADER_H_

#include <GLES2/gl2.h>
#include <map>
#include <string>

namespace gpu {

class Texture;

class Shader {
 public:
  Shader();
  ~Shader();

  void Init(const char* vertex_shader, const char* frag_shader);

  GLuint GetAttribLocation(const char* name);
  GLuint GetUniformLocation(const char* name);
  void Use();
  void Uniform1f(const char* name, GLfloat value);
  void Uniform1i(const char* name, GLint value);
  void UniformTexture(const char* name, int index, const Texture& texture);
  void UniformMatrixOrtho(const char* name, float l, float r, float b, float t,
                          float near, float far);

 private:
  GLuint CompileShader(GLenum type, const char* data);
  GLuint MakeProgram(const char* vertex_shader, const char* frag_shader);

  GLuint id_;
  typedef std::map<std::string, GLuint> NameLocationMap;
  NameLocationMap attribs_;
  NameLocationMap uniforms_;
};

}  // namespace gpu

#endif  // GPU_SHADER_H_
