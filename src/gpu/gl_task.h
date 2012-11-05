// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_GL_TASK_H_
#define GPU_GL_TASK_H_

#include <functional>
#include <memory>
#include <stdint.h>
#include "gpu/wrap_gl.h"

namespace gpu {

typedef std::shared_ptr<uint8_t> UniqueData;
typedef std::shared_ptr<float> UniqueMatrix;

// Wrapped functions.
void task_glAttachShader(ID program, ID shader);
void task_glBindBuffer(GLenum target, ID buffer);
void task_glBindFramebuffer(GLenum target, ID framebuffer);
void task_glBindTexture(GLenum target, ID texture);
void task_glBufferData(GLenum target, GLsizeiptr size, UniqueData data,
                       GLenum usage);
void task_glCompileShader(ID shader);
void task_glCreateProgram(ID out_program);
void task_glCreateShader(ID out_shader, GLenum type);
void task_glDeleteProgram(ID program);
void task_glDeleteShader(ID shader);
void task_glDeleteFramebuffer(ID framebuffer);
void task_glDeleteTexture(ID texture);
void task_glEnableVertexAttribArray(Location index);
void task_glFramebufferTexture2D(GLenum target, GLenum attachment,
                                 GLenum textarget, ID texture, GLint level);
void task_glGenBuffer(ID buffer);
void task_glGenFramebuffer(ID framebuffer);
void task_glGenTexture(ID texture);
void task_glGetAttribLocation(Location out_location, ID program,
                              const GLchar* name);
void task_glGetUniformLocation(Location out_location, ID program,
                               const GLchar* name);
void task_glLinkProgram(ID program);
void task_glShaderSource(ID shader, GLsizei count, const GLchar** string,
                         const GLint* length);
void task_glTexImage2D(GLenum target, GLint level, GLint internalformat,
                       GLsizei width, GLsizei height, GLint border,
                       GLenum format, GLenum type, UniqueData pixels);
void task_glUniform1f(Location location, GLfloat x);
void task_glUniform1i(Location location, GLint x);
void task_glUniformMatrix4fv(Location location, GLsizei count,
                             GLboolean transpose, UniqueMatrix value);
void task_glUseProgram(ID program);
void task_glVertexAttribPointer(Location indx, GLint size, GLenum type,
                                GLboolean normalized, GLsizei stride,
                                const GLvoid* ptr);
void task_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);


class GLTask {
 public:
  virtual ~GLTask() {}
  virtual void Run() = 0;
};

class FunctionGLTask : public GLTask {
 public:
  typedef void FunctionType();
  explicit FunctionGLTask(const std::function<FunctionType>& function);
  virtual void Run();

 private:
  std::function<FunctionType> function_;
};

}  // namespace gpu

#endif  // GPU_GL_TASK_H_
