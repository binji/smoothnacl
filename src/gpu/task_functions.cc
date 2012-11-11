// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/task_functions.h"
#include "gpu/future.h"
#include <stdio.h>

//#define printf(...)

namespace gpu {

void task_glAttachShader(ID program, ID shader) {
  printf("glAttachShader(%d, %d)\n", program.value(), shader.value());
  ::glAttachShader(program.value(), shader.value());
}

void task_glBindBuffer(GLenum target, ID buffer) {
  printf("glBindBuffer(%d, %d)\n", target, buffer.value());
  ::glBindBuffer(target, buffer.value());
}

void task_glBindFramebuffer(GLenum target, ID framebuffer) {
  printf("glBindFramebuffer(%d, %d)\n", target, framebuffer.value());
  ::glBindFramebuffer(target, framebuffer.value());
}

void task_glBindTexture(GLenum target, ID texture) {
  printf("glBindTexture(%d, %d)\n", target, texture.value());
  ::glBindTexture(target, texture.value());
}

void task_glBufferData(GLenum target, GLsizeiptr size, UniqueData data,
                       GLenum usage) {
  printf("glBufferData(%d, %d, %p, %d)\n", target, size, data.get(), usage);
  printf("  { ");
  float* fdata = reinterpret_cast<float*>(data.get());
  for (int i = 0; i < 4 * 9; ++i)
    printf("%.2f,", fdata[i]);
  printf("  }\n");
  ::glBufferData(target, size, data.get(), usage);
}

void task_glCompileShader(ID shader) {
  printf("glCompileShader(%d)\n", shader.value());
  ::glCompileShader(shader.value());
}

void task_glCreateProgram(ID out_program) {
  GLuint id = ::glCreateProgram();
  out_program.set_value(id);
  printf("glCreateProgram() = %d\n", id);
}

void task_glCreateShader(ID out_shader, GLenum type) {
  GLuint id = ::glCreateShader(type);
  out_shader.set_value(id);
  printf("glCreateShader(%d) = %d\n", type, id);
}

void task_glDeleteProgram(ID program) {
  printf("glDeleteProgram(%d)\n", program.value());
  ::glDeleteProgram(program.value());
}

void task_glDeleteShader(ID shader) {
  printf("glDeleteShader(%d)\n", shader.value());
  ::glDeleteShader(shader.value());
}

void task_glDeleteFramebuffer(ID framebuffer) {
  printf("glDeleteFramebuffer(%d)\n", framebuffer.value());
  GLuint id = framebuffer.value();
  ::glDeleteFramebuffers(1, &id);
}

void task_glDeleteTexture(ID texture) {
  printf("glDeleteTexture(%d)\n", texture.value());
  GLuint id = texture.value();
  ::glDeleteTextures(1, &id);
}

void task_glEnableVertexAttribArray(Location index) {
  printf("glEnableVertexAttribArray(%d)\n", index.value());
  ::glEnableVertexAttribArray(index.value());
}

void task_glFramebufferTexture2D(GLenum target, GLenum attachment,
                                 GLenum textarget, ID texture, GLint level) {
  printf("glFramebufferTexture2D(%d, %d, %d, %d, %d)\n", target, attachment, textarget, texture.value(), level);
  ::glFramebufferTexture2D(target, attachment, textarget, texture.value(),
                           level);
}

void task_glGenBuffer(ID buffer) {
  GLuint id;
  ::glGenBuffers(1, &id);
  buffer.set_value(id);
  printf("glGenBuffer() = %d\n", id);
}

void task_glGenFramebuffer(ID framebuffer) {
  GLuint id;
  ::glGenFramebuffers(1, &id);
  framebuffer.set_value(id);
  printf("glGenFramebuffer() = %d\n", id);
}

void task_glGenTexture(ID texture) {
  GLuint id;
  ::glGenTextures(1, &id);
  texture.set_value(id);
  printf("glGenTexture() = %d\n", id);
}

void task_glGetAttribLocation(Location out_location, ID program,
                              const GLchar* name) {
  GLint location = ::glGetAttribLocation(program.value(), name);
  out_location.set_value(location);
  printf("glGetAttribLocation() = %d\n", location);
}

void task_glGetUniformLocation(Location out_location, ID program,
                               const GLchar* name) {
  GLint location = ::glGetUniformLocation(program.value(), name);
  out_location.set_value(location);
  printf("glGetUniformLocation() = %d\n", location);
}

void task_glLinkProgram(ID program) {
  printf("glLinkProgram(%d)\n", program.value());
  ::glLinkProgram(program.value());

  char buffer[4096];
  GLsizei length;
  glGetProgramInfoLog(program.value(), 4096, &length, &buffer[0]);
  buffer[length] = 0;
  printf("progLog: %s\n", buffer);
}

void task_glShaderSource(ID shader, GLsizei count, UniqueStrings strings,
                         const GLint* length) {
  printf("glShaderSource(%d, %d, %p, %p)\n", shader.value(), count, strings.get(), length);
  ::glShaderSource(shader.value(), count, strings.get(), length);

  char buffer[4096];
  GLsizei buflen;
  glGetShaderInfoLog(shader.value(), 4096, &buflen, &buffer[0]);
  buffer[buflen] = 0;
  printf("shaderLog: %s\n", buffer);
}

void task_glTexImage2D(GLenum target, GLint level, GLint internalformat,
                       GLsizei width, GLsizei height, GLint border,
                       GLenum format, GLenum type, UniqueData pixels) {
  printf("glTexImage2D(%d, %d, %d, %d, %d, %d, %d, %d, %p)\n", target, level, internalformat, width, height, border, format, type, pixels.get());
  ::glTexImage2D(target, level, internalformat, width, height, border, format,
                 type, pixels.get());
  if (pixels.get()) {
    printf("  { ");
    float* fdata = reinterpret_cast<float*>(pixels.get());
    for (int i = 0; i < 100; ++i)
      printf("%.2f,", fdata[i]);
    printf("...\n");
  }
}

void task_glUniform1f(Location location, GLfloat x) {
  printf("glUniform1f(%d, %.3f)\n", location.value(), x);
  ::glUniform1f(location.value(), x);
}

void task_glUniform1i(Location location, GLint x) {
  printf("glUniform1f(%d, %d)\n", location.value(), x);
  ::glUniform1i(location.value(), x);
}

void task_glUniformMatrix4fv(Location location, GLsizei count,
                             GLboolean transpose, UniqueMatrix value) {
  printf("glUniformMatrix4fv(%d, %d, %d, %p)\n", location.value(), count, transpose, value.get());
  printf("  { ");
  float* fdata = value.get();
  for (int i = 0; i < 4 * 4; ++i)
    printf("%.2f,", fdata[i]);
  printf("  }\n");
  ::glUniformMatrix4fv(location.value(), count, transpose, value.get());
}

void task_glUseProgram(ID program) {
  printf("glUseProgram(%d)\n", program.value());
  ::glUseProgram(program.value());
}

void task_glVertexAttribPointer(Location indx, GLint size, GLenum type,
                                GLboolean normalized, GLsizei stride,
                                const GLvoid* ptr) {
  printf("glVertexAttribPointer(%d, %d, %d, %d, %d, %p)\n", indx.value(), size, type, normalized, stride, ptr);
  ::glVertexAttribPointer(indx.value(), size, type, normalized, stride, ptr);
}

}  // namespace gpu
