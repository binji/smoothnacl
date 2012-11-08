// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/wrap_gl.h"
#include <assert.h>
#include <functional>
#include <memory>
#include <string.h>
#include <stdio.h>
#include "gpu/gl_task.h"
#include "gpu/task_functions.h"

namespace gpu {
namespace {

template <typename T>
struct ArrayDeleter {
  void operator ()(T* t) {
    delete [] t;
  }
};

template <typename T>
std::shared_ptr<T> CloneData(const void* data, size_t size) {
  if (data == NULL)
    return std::shared_ptr<T>();

  T* new_data = new T[size];
  memcpy(new_data, data, size * sizeof(T));
  return std::shared_ptr<T>(new_data, ArrayDeleter<T>());
}

}  // namespace

void glActiveTexture(GLenum texture) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&::glActiveTexture, texture)));
}

void glAttachShader(ID program, ID shader) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glAttachShader, program, shader)));
}

void glBindBuffer(GLenum target, ID buffer) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glBindBuffer, target, buffer)));
}

void glBindFramebuffer(GLenum target, ID framebuffer) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glBindFramebuffer, target, framebuffer)));
}

void glBindTexture(GLenum target, ID texture) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glBindTexture, target, texture)));
}

void glBufferData(GLenum target, GLsizeiptr size, const GLvoid* data,
                  GLenum usage) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glBufferData, target, size,
                std::move(CloneData<uint8_t>(data, size)), usage)));
}

void glClear(GLbitfield mask) {
  g_task_list.Enqueue(new FunctionGLTask(std::bind(&::glClear, mask)));
}

void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&::glClearColor, red, green, blue, alpha)));
}

void glCompileShader(ID shader) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glCompileShader, shader)));
}

ID glCreateProgram(void) {
  ID id;
  g_task_list.Enqueue(new FunctionGLTask(std::bind(&task_glCreateProgram, id)));
  return id;
}

ID glCreateShader(GLenum type) {
  ID id;
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glCreateShader, id, type)));
  return id;
}

void glDeleteProgram(ID program) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glDeleteProgram, program)));
}

void glDeleteFramebuffers(GLsizei n, const ID* framebuffers) {
  assert(n == 1);
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glDeleteFramebuffer, framebuffers[0])));
}

void glDeleteShader(ID shader) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glDeleteShader, shader)));
}

void glDeleteTextures(GLsizei n, const ID* textures) {
  assert(n == 1);
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glDeleteShader, textures[0])));
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&::glDrawArrays, mode, first, count)));
}

void glEnableVertexAttribArray(Location index) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glEnableVertexAttribArray, index)));
}

void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget,
                            ID texture, GLint level) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glFramebufferTexture2D, target, attachment, textarget,
                texture, level)));
}

void glGenBuffers(GLsizei n, ID* buffers) {
  assert(n == 1);
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glGenBuffer, buffers[0])));
}

void glGenFramebuffers(GLsizei n, ID* framebuffers) {
  assert(n == 1);
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glGenFramebuffer, framebuffers[0])));
}

void glGenTextures(GLsizei n, ID* textures) {
  assert(n == 1);
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glGenTexture, textures[0])));
}

Location glGetAttribLocation(ID program, const GLchar* name) {
  Location loc;
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glGetAttribLocation, loc, program, name)));
  return loc;
}

Location glGetUniformLocation(ID program, const GLchar* name) {
  Location loc;
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glGetUniformLocation, loc, program, name)));
  return loc;
}

void glLinkProgram(ID program) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glLinkProgram, program)));
}

void glShaderSource(ID shader, GLsizei count, const GLchar** string,
                    const GLint* length) {
  assert(length == NULL);
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glShaderSource, shader, count,
                std::move(CloneData<const char*>(string, count)), length)));
}

void glTexImage2D(GLenum target, GLint level, GLint internalformat,
                  GLsizei width, GLsizei height, GLint border, GLenum format,
                  GLenum type, const GLvoid* pixels) {
  assert(type == GL_FLOAT);
  size_t bpp;
  switch (format) {
    case GL_LUMINANCE: bpp = 4; break;
    case GL_RGBA: bpp = 16; break;
    default:
      printf("Unknown format: %d\n", format);
      assert(0);
      break;
  }
  size_t size = width * height * bpp;

  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glTexImage2D, target, level, internalformat, width,
                height, border, format, type,
                std::move(CloneData<uint8_t>(pixels, size)))));
}

void glTexParameterf(GLenum target, GLenum pname, GLfloat param) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&::glTexParameterf, target, pname, param)));
}

void glUniform1f(Location location, GLfloat x) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glUniform1f, location, x)));
}

void glUniform1i(Location location, GLint x) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glUniform1i, location, x)));
}

void glUniformMatrix4fv(Location location, GLsizei count, GLboolean transpose,
                        const GLfloat* value) {
  assert(count == 1);
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glUniformMatrix4fv, location, count, transpose,
                std::move(CloneData<float>(value, 16)))));
}

void glUseProgram(ID program) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glUseProgram, program)));
}

void glVertexAttribPointer(Location indx, GLint size, GLenum type,
                           GLboolean normalized, GLsizei stride,
                           const GLvoid* ptr) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&task_glVertexAttribPointer, indx, size, type,
                normalized, stride, ptr)));
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
  g_task_list.Enqueue(new FunctionGLTask(
      std::bind(&::glViewport, x, y, width, height)));
}

}  // namespace gpu
