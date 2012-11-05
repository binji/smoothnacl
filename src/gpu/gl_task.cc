// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/gl_task.h"
#include <GLES2/gl2.h>

namespace gpu {

void task_glAttachShader(ID program, ID shader) {
  ::glAttachShader(program.value(), shader.value());
}

void task_glBindBuffer(GLenum target, ID buffer) {
  ::glBindBuffer(target, buffer.value());
}

void task_glBindFramebuffer(GLenum target, ID framebuffer) {
  ::glBindFramebuffer(target, framebuffer.value());
}

void task_glBindTexture(GLenum target, ID texture) {
  ::glBindTexture(target, texture.value());
}

void task_glBufferData(GLenum target, GLsizeiptr size, const GLvoid* data,
                       GLenum usage) {
  ::glBufferData(target, size, data, usage);
}

void task_glCompileShader(ID shader) {
  ::glCompileShader(shader.value());
}

void task_glCreateProgram(ID* out_program) {
  GLuint id = ::glCreateProgram();
  out_program->set_value(id);
}

void task_glCreateShader(ID* out_shader, GLenum type) {
  GLuint id = ::glCreateShader(type);
  out_shader->set_value(id);
}

void task_glDeleteProgram(ID program) {
  ::glDeleteProgram(program.value());
}

void task_glDeleteShader(ID shader) {
  ::glDeleteShader(shader.value());
}

void task_glDeleteFramebuffer(ID framebuffer) {
  GLuint id = framebuffer.value();
  ::glDeleteFramebuffers(1, &id);
}

void task_glDeleteTexture(ID texture) {
  GLuint id = texture.value();
  ::glDeleteTextures(1, &id);
}

void task_glEnableVertexAttribArray(Location index) {
  ::glEnableVertexAttribArray(index.value());
}

void task_glFramebufferTexture2D(GLenum target, GLenum attachment,
                                 GLenum textarget, ID texture, GLint level) {
  ::glFramebufferTexture2D(target, attachment, textarget, texture.value(),
                           level);
}

void task_glGenBuffers(GLsizei n, ID* buffers) {
  assert(n == 1);
  GLuint id;
  ::glGenBuffers(1, &id);
  buffers[0].set_value(id);
}

void task_glGenFramebuffers(GLsizei n, ID* framebuffers) {
  assert(n == 1);
  GLuint id;
  ::glGenFramebuffers(1, &id);
  framebuffers[0].set_value(id);
}

void task_glGenTextures(GLsizei n, ID* textures) {
  assert(n == 1);
  GLuint id;
  ::glGenTextures(1, &id);
  textures[0].set_value(id);
}

void task_glGetAttribLocation(Location* out_location, ID program,
                              const GLchar* name) {
  GLint location = ::glGetAttribLocation(program.value(), name);
  out_location->set_value(location);
}

void task_glGetUniformLocation(Location* out_location, ID program,
                               const GLchar* name) {
  GLint location = ::glGetUniformLocation(program.value(), name);
  out_location->set_value(location);
}

void task_glLinkProgram(ID program) {
  ::glLinkProgram(program.value());
}

void task_glShaderSource(ID shader, GLsizei count, const GLchar** string,
                         const GLint* length) {
  ::glShaderSource(shader.value(), count, string, length);
}

void task_glTexImage2D(GLenum target, GLint level, GLint internalformat,
                       GLsizei width, GLsizei height, GLint border,
                       GLenum format, GLenum type, const GLvoid* pixels) {
  ::glTexImage2D(target, level, internalformat, width, height, border, format,
                 type, pixels);
}

void task_glUniform1f(Location location, GLfloat x) {
  ::glUniform1f(location.value(), x);
}

void task_glUniform1i(Location location, GLint x) {
  ::glUniform1i(location.value(), x);
}

void task_glUniformMatrix4fv(Location location, GLsizei count,
                             GLboolean transpose, const GLfloat* value) {
  ::glUniformMatrix4fv(location.value(), count, transpose, value);
}

void task_glUseProgram(ID program) {
  ::glUseProgram(program.value());
}

void task_glVertexAttribPointer(Location indx, GLint size, GLenum type,
                                GLboolean normalized, GLsizei stride,
                                const GLvoid* ptr) {
  ::glVertexAttribPointer(indx.value(), size, type, normalized, stride, ptr);
}

FunctionGLTask::FunctionGLTask(const std::function<FunctionType>& function)
    : function_(function) {
}

void FunctionGLTask::Run() {
  function_();
}

}  // namespace gpu
