// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/shader.h"

namespace gpu {

Shader::Shader(const char* vertex_shader, const char* frag_shader) {
  id_ = MakeProgram(vertex_shader, frag_shader);
}

Shader::~Shader() {
}

GLuint Shader::GetAttribLocation(const char* name) {
  NameLocationMap::iterator iter = attribs_.find(name);
  if (iter != attribs_.end())
    return iter->second;

  GLuint location = glGetAttribLocation(id_, name);
  attribs_[name] = location;
  return location;
}

GLuint Shader::GetUniformLocation(const char* name) {
  NameLocationMap::iterator iter = uniforms_.find(name);
  if (iter != uniforms_.end())
    return iter->second;

  GLuint location = glGetUniformLocation(id_, name);
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

GLuint Shader::CompileShader(GLenum type, const char* data) {
  const char* shader_strings[1];
  shader_strings[0] = data;
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, shader_strings, NULL);
  glCompileShader(shader);
  return shader;
}

GLuint Shader::MakeProgram(const char* vertex_shader, const char* frag_shader) {
  GLuint vert = CompileShader(GL_VERTEX_SHADER, vertex_shader);
  GLuint frag = CompileShader(GL_FRAGMENT_SHADER, frag_shader);
  GLuint prog = glCreateProgram();
  glAttachShader(prog, vert);
  glAttachShader(prog, frag);
  glLinkProgram(prog);
  return prog;
}

}  // namespace gpu
