// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/wrap_gl.h"
#include <assert.h>
#include <functional>
#include "gpu/gl_task.h"

namespace gpu {
namespace {

// TODO(binji): implement
void EnqueueTask(GLTask*) {
}

}  // namespace

void glActiveTexture(GLenum texture) {
  EnqueueTask(new FunctionGLTask(std::bind(&::glActiveTexture, texture)));
}

void glAttachShader(ID program, ID shader) {
  EnqueueTask(new FunctionGLTask(
      std::bind(&task_glAttachShader, program, shader)));
}

void glBindBuffer(GLenum target, ID buffer) {
  EnqueueTask(new FunctionGLTask(
      std::bind(&task_glBindBuffer, target, buffer)));
}

void glBindFramebuffer(GLenum target, ID framebuffer) {
  EnqueueTask(new FunctionGLTask(
      std::bind(&task_glBindFramebuffer, target, framebuffer)));
}

void glBindTexture(GLenum target, ID texture) {
  EnqueueTask(new FunctionGLTask(
      std::bind(&task_glBindTexture, target, texture)));
}

void glBufferData(GLenum target, GLsizeiptr size, const GLvoid* data,
                  GLenum usage) {
  // TODO(binji): clone buffer
  EnqueueTask(new FunctionGLTask(
      std::bind(&task_glBufferData, target, size, data, usage)));
}

void glClear(GLbitfield mask) {
  EnqueueTask(new FunctionGLTask(std::bind(&::glClear, mask)));
}

void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
  EnqueueTask(new FunctionGLTask(
      std::bind(&::glClearColor, red, green, blue, alpha)));
}

void glCompileShader(ID shader) {
  EnqueueTask(new FunctionGLTask(std::bind(&task_glCompileShader, shader)));
}

ID glCreateProgram(void) {
  ID id;
  EnqueueTask(new FunctionGLTask(std::bind(&task_glCreateProgram, &id)));
  return id;
}

ID glCreateShader(GLenum type) {
  ID id;
  EnqueueTask(new FunctionGLTask(std::bind(&task_glCreateShader, &id, type)));
  return id;
}

void glDeleteProgram(ID program) {
  EnqueueTask(new FunctionGLTask(std::bind(&task_glDeleteProgram, program)));
}

void glDeleteFramebuffers(GLsizei n, const ID* framebuffers) {
  assert(n == 1);
  EnqueueTask(new FunctionGLTask(
      std::bind(&task_glDeleteFramebuffer, framebuffers[0])));
}

void glDeleteShader(ID shader) {
  EnqueueTask(new FunctionGLTask(std::bind(&task_glDeleteShader, shader)));
}

void glDeleteTextures(GLsizei n, const ID* textures) {
  assert(n == 1);
  EnqueueTask(new FunctionGLTask(
      std::bind(&task_glDeleteShader, textures[0])));
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
  EnqueueTask(new FunctionGLTask(
      std::bind(&::glDrawArrays, mode, first, count)));
}

void glEnableVertexAttribArray(Location index) {
  EnqueueTask(new FunctionGLTask(
      std::bind(&task_glEnableVertexAttribArray, index)));
}

void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget,
                            ID texture, GLint level) {
  EnqueueTask(new FunctionGLTask(
      std::bind(&task_glFramebufferTexture2D, target, attachment, textarget,
                texture, level)));
}

void glGenBuffers(GLsizei n, ID* buffers) {
  assert(n == 1);
  EnqueueTask(new FunctionGLTask(
      std::bind(&task_glGenBuffers, 1, &buffers[0])));
}

void glGenFramebuffers(GLsizei n, ID* framebuffers) {
  assert(n == 1);
  EnqueueTask(new FunctionGLTask(
      std::bind(&task_glGenFramebuffers, 1, &framebuffers[0])));
}

void glGenTextures(GLsizei n, ID* textures) {
  assert(n == 1);
  EnqueueTask(new FunctionGLTask(
      std::bind(&task_glGenTextures, 1, &textures[0])));
}

Location glGetAttribLocation(ID program, const GLchar* name) {
  Location loc;
  EnqueueTask(new FunctionGLTask(
      std::bind(&task_glGetAttribLocation, &loc, program, name)));
  return loc;
}

Location glGetUniformLocation(ID program, const GLchar* name) {
  Location loc;
  EnqueueTask(new FunctionGLTask(
      std::bind(&task_glGetUniformLocation, &loc, program, name)));
  return loc;
}

void glLinkProgram(ID program) {
  EnqueueTask(new FunctionGLTask(std::bind(&task_glLinkProgram, program)));
}

void glShaderSource(ID shader, GLsizei count, const GLchar** string,
                    const GLint* length) {
  EnqueueTask(new FunctionGLTask(
      std::bind(&task_glShaderSource, shader, count, string, length)));
}

void glTexImage2D(GLenum target, GLint level, GLint internalformat,
                  GLsizei width, GLsizei height, GLint border, GLenum format,
                  GLenum type, const GLvoid* pixels) {
  // TODO(binji): clone buffer
  EnqueueTask(new FunctionGLTask(
      std::bind(&task_glTexImage2D, target, level, internalformat, width,
                height, border, format, type, pixels)));
}

void glTexParameterf(GLenum target, GLenum pname, GLfloat param) {
  EnqueueTask(new FunctionGLTask(
      std::bind(&::glTexParameterf, target, pname, param)));
}

void glUniform1f(Location location, GLfloat x) {
  EnqueueTask(new FunctionGLTask(std::bind(&task_glUniform1f, location, x)));
}

void glUniform1i(Location location, GLint x) {
  EnqueueTask(new FunctionGLTask(std::bind(&task_glUniform1i, location, x)));
}

void glUniformMatrix4fv(Location location, GLsizei count, GLboolean transpose,
                        const GLfloat* value) {
  // TODO(binji): clone buffer
  EnqueueTask(new FunctionGLTask(
      std::bind(&task_glUniformMatrix4fv, location, count, transpose, value)));
}

void glUseProgram(ID program) {
  EnqueueTask(new FunctionGLTask(std::bind(&task_glUseProgram, program)));
}

void glVertexAttribPointer(Location indx, GLint size, GLenum type,
                           GLboolean normalized, GLsizei stride,
                           const GLvoid* ptr) {
  EnqueueTask(new FunctionGLTask(
      std::bind(&task_glVertexAttribPointer, indx, size, type,
                normalized, stride, ptr)));
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
  EnqueueTask(new FunctionGLTask(
      std::bind(&::glViewport, x, y, width, height)));
}


}  // namespace gpu
