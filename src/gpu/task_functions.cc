// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/task_functions.h"
#include "gpu/future.h"
#include <stdio.h>

#define LOG_GL(...)
#if 1
#define CHECK_ERROR
#else
#define CHECK_ERROR \
  do { \
    if (glGetError()) { \
      printf("glGetError returned non-zero. %s:%d\n", __FILE__, __LINE__); \
    } \
  } while(0)  // no semicolon
#endif

namespace gpu {

void task_glAttachShader(ID program, ID shader) {
  LOG_GL("glAttachShader(%d, %d)\n", program.value(), shader.value());
  ::glAttachShader(program.value(), shader.value());
  CHECK_ERROR;
}

void task_glBindBuffer(GLenum target, ID buffer) {
  LOG_GL("glBindBuffer(%d, %d)\n", target, buffer.value());
  ::glBindBuffer(target, buffer.value());
  CHECK_ERROR;
}

void task_glBindFramebuffer(GLenum target, ID framebuffer) {
  LOG_GL("glBindFramebuffer(%d, %d)\n", target, framebuffer.value());
  ::glBindFramebuffer(target, framebuffer.value());
  CHECK_ERROR;
}

void task_glBindTexture(GLenum target, ID texture) {
  LOG_GL("glBindTexture(%d, %d)\n", target, texture.value());
  ::glBindTexture(target, texture.value());
  CHECK_ERROR;
}

void task_glBufferData(GLenum target, GLsizeiptr size, UniqueData data,
                       GLenum usage) {
  LOG_GL("glBufferData(%d, %d, %p, %d)\n", target, size, data.get(), usage);
  LOG_GL("  { ");
  float* fdata = reinterpret_cast<float*>(data.get());
  for (int i = 0; i < 4 * 9; ++i)
    LOG_GL("%.2f,", fdata[i]);
  LOG_GL("  }\n");
  ::glBufferData(target, size, data.get(), usage);
  CHECK_ERROR;
}

void task_glCompileShader(ID shader) {
  LOG_GL("glCompileShader(%d)\n", shader.value());
  ::glCompileShader(shader.value());
  CHECK_ERROR;
}

void task_glCreateProgram(ID out_program) {
  GLuint id = ::glCreateProgram();
  out_program.set_value(id);
  LOG_GL("glCreateProgram() = %d\n", id);
  CHECK_ERROR;
}

void task_glCreateShader(ID out_shader, GLenum type) {
  GLuint id = ::glCreateShader(type);
  out_shader.set_value(id);
  LOG_GL("glCreateShader(%d) = %d\n", type, id);
  CHECK_ERROR;
}

void task_glDeleteProgram(ID program) {
  LOG_GL("glDeleteProgram(%d)\n", program.value());
  ::glDeleteProgram(program.value());
  CHECK_ERROR;
}

void task_glDeleteShader(ID shader) {
  LOG_GL("glDeleteShader(%d)\n", shader.value());
  ::glDeleteShader(shader.value());
  CHECK_ERROR;
}

void task_glDeleteFramebuffer(ID framebuffer) {
  LOG_GL("glDeleteFramebuffer(%d)\n", framebuffer.value());
  GLuint id = framebuffer.value();
  ::glDeleteFramebuffers(1, &id);
  CHECK_ERROR;
}

void task_glDeleteTexture(ID texture) {
  LOG_GL("glDeleteTexture(%d)\n", texture.value());
  GLuint id = texture.value();
  ::glDeleteTextures(1, &id);
  CHECK_ERROR;
}

void task_glEnableVertexAttribArray(Location index) {
  LOG_GL("glEnableVertexAttribArray(%d)\n", index.value());
  ::glEnableVertexAttribArray(index.value());
  CHECK_ERROR;
}

void task_glFramebufferTexture2D(GLenum target, GLenum attachment,
                                 GLenum textarget, ID texture, GLint level) {
  LOG_GL("glFramebufferTexture2D(%d, %d, %d, %d, %d)\n", target, attachment, textarget, texture.value(), level);
  ::glFramebufferTexture2D(target, attachment, textarget, texture.value(),
                           level);
  CHECK_ERROR;
}

void task_glGenBuffer(ID buffer) {
  GLuint id;
  ::glGenBuffers(1, &id);
  buffer.set_value(id);
  LOG_GL("glGenBuffer() = %d\n", id);
  CHECK_ERROR;
}

void task_glGenFramebuffer(ID framebuffer) {
  GLuint id;
  ::glGenFramebuffers(1, &id);
  framebuffer.set_value(id);
  LOG_GL("glGenFramebuffer() = %d\n", id);
  CHECK_ERROR;
}

void task_glGenTexture(ID texture) {
  GLuint id;
  ::glGenTextures(1, &id);
  texture.set_value(id);
  LOG_GL("glGenTexture() = %d\n", id);
  CHECK_ERROR;
}

void task_glGetAttribLocation(Location out_location, ID program,
                              const GLchar* name) {
  GLint location = ::glGetAttribLocation(program.value(), name);
  out_location.set_value(location);
  LOG_GL("glGetAttribLocation() = %d\n", location);
  CHECK_ERROR;
}

void task_glGetUniformLocation(Location out_location, ID program,
                               const GLchar* name) {
  GLint location = ::glGetUniformLocation(program.value(), name);
  out_location.set_value(location);
  LOG_GL("glGetUniformLocation() = %d\n", location);
  CHECK_ERROR;
}

void task_glLinkProgram(ID program) {
  LOG_GL("glLinkProgram(%d)\n", program.value());
  ::glLinkProgram(program.value());

  char buffer[4096];
  GLsizei length;
  glGetProgramInfoLog(program.value(), 4096, &length, &buffer[0]);
  buffer[length] = 0;
  LOG_GL("progLog: %s\n", buffer);
  CHECK_ERROR;
}

void task_glShaderSource(ID shader, GLsizei count, UniqueStrings strings,
                         const GLint* length) {
  LOG_GL("glShaderSource(%d, %d, %p, %p)\n", shader.value(), count, strings.get(), length);
  ::glShaderSource(shader.value(), count, strings.get(), length);

  char buffer[4096];
  GLsizei buflen;
  glGetShaderInfoLog(shader.value(), 4096, &buflen, &buffer[0]);
  buffer[buflen] = 0;
  LOG_GL("shaderLog: %s\n", buffer);
  CHECK_ERROR;
}

void task_glTexImage2D(GLenum target, GLint level, GLint internalformat,
                       GLsizei width, GLsizei height, GLint border,
                       GLenum format, GLenum type, UniqueData pixels) {
  LOG_GL("glTexImage2D(%d, %d, %d, %d, %d, %d, %d, %d, %p)\n", target, level, internalformat, width, height, border, format, type, pixels.get());
  ::glTexImage2D(target, level, internalformat, width, height, border, format,
                 type, pixels.get());
  if (pixels.get()) {
    LOG_GL("  { ");
    float* fdata = reinterpret_cast<float*>(pixels.get());
    for (int i = 0; i < 100; ++i)
      LOG_GL("%.2f,", fdata[i]);
    LOG_GL("...\n");
  }
  CHECK_ERROR;
}

void task_glUniform1f(Location location, GLfloat x) {
  LOG_GL("glUniform1f(%d, %.3f)\n", location.value(), x);
  ::glUniform1f(location.value(), x);
  CHECK_ERROR;
}

void task_glUniform1i(Location location, GLint x) {
  LOG_GL("glUniform1i(%d, %d)\n", location.value(), x);
  ::glUniform1i(location.value(), x);
  CHECK_ERROR;
}

void task_glUniformMatrix4fv(Location location, GLsizei count,
                             GLboolean transpose, UniqueMatrix value) {
  LOG_GL("glUniformMatrix4fv(%d, %d, %d, %p)\n", location.value(), count, transpose, value.get());
  LOG_GL("  { ");
  float* fdata = value.get();
  for (int i = 0; i < 4 * 4; ++i)
    LOG_GL("%.2f,", fdata[i]);
  LOG_GL("  }\n");
  ::glUniformMatrix4fv(location.value(), count, transpose, value.get());
  CHECK_ERROR;
}

void task_glUseProgram(ID program) {
  LOG_GL("glUseProgram(%d)\n", program.value());
  ::glUseProgram(program.value());
  CHECK_ERROR;
}

void task_glVertexAttribPointer(Location indx, GLint size, GLenum type,
                                GLboolean normalized, GLsizei stride,
                                const GLvoid* ptr) {
  LOG_GL("glVertexAttribPointer(%d, %d, %d, %d, %d, %p)\n", indx.value(), size, type, normalized, stride, ptr);
  ::glVertexAttribPointer(indx.value(), size, type, normalized, stride, ptr);
  CHECK_ERROR;
}

}  // namespace gpu
