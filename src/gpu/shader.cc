// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/shader.h"
#include "gpu/texture.h"

namespace gpu {

Shader::Shader() {
}

Shader::~Shader() {
}

void Shader::Init(const char* frag_shader, const char* vertex_shader) {
  vert_id_ = CompileShader(GL_VERTEX_SHADER, vertex_shader);
  frag_id_ = CompileShader(GL_FRAGMENT_SHADER, frag_shader);
  id_ = glCreateProgram();
  glAttachShader(id_, vert_id_);
  glAttachShader(id_, frag_id_);
  glLinkProgram(id_);
}

Location Shader::GetAttribLocation(const char* name) {
  NameLocationMap::iterator iter = attribs_.find(name);
  if (iter != attribs_.end())
    return iter->second;

  PassLocation location = glGetAttribLocation(id_, name);
  attribs_[name] = location;
  return location;
}

Location Shader::GetUniformLocation(const char* name) {
  NameLocationMap::iterator iter = uniforms_.find(name);
  if (iter != uniforms_.end())
    return iter->second;

  PassLocation location = glGetUniformLocation(id_, name);
  uniforms_[name] = location;
  return location;
}

void Shader::Use() {
  glUseProgram(id_);
}

void Shader::Uniform1f(const char* name, GLfloat value) {
  glUniform1f(GetUniformLocation(name), value);
}

void Shader::Uniform1i(const char* name, GLint value) {
  glUniform1i(GetUniformLocation(name), value);
}

void Shader::UniformTexture(const char* name, int index,
                            const Texture& texture) {
  glActiveTexture(GL_TEXTURE0 + index);
  glBindTexture(GL_TEXTURE_2D, texture.id());
  glUniform1i(GetUniformLocation(name), index);
}

void Shader::UniformMatrixOrtho(const char* name, float l, float r, float b,
                                float t, float near, float far) {
  float matrix[16] = {0};
  matrix[0] = 2.0f / (r - l);
  matrix[5] = 2.0f / (t - b);
  matrix[10] = -2.0f / (far - near);
  matrix[12] = -(r + l) / (r - l);
  matrix[13] = -(t + b) / (t - b);
  matrix[14] = -(far + near) / (far - near);
  matrix[15] = 1;
  glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0]);
}

PassID Shader::CompileShader(GLenum type, const char* data) {
  const char* shader_strings[1];
  shader_strings[0] = data;
  ID shader = glCreateShader(type);
  glShaderSource(shader, 1, shader_strings, NULL);
  glCompileShader(shader);
  return shader;
}

}  // namespace gpu
