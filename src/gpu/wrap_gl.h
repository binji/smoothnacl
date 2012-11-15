// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_WRAP_GL_H_
#define GPU_WRAP_GL_H_

#include <GLES2/gl2.h>
#include "gpu/future.h"

// First, undefine all GL calls.
#include "gpu/undef_gl.h"

// Redefine all GL calls, even though we only wrap a subset; that way if new
// calls are added we break at compile time.

#undef GLES2_GET_FUN
#define GLES2_GET_FUN(F) wrap_gl ## F
#include "gpu/define_gl.h"

namespace gpu {

void wrap_glActiveTexture(GLenum texture);
void wrap_glAttachShader(ID program, ID shader);
void wrap_glBindBuffer(GLenum target, ID buffer);
void wrap_glBindFramebuffer(GLenum target, ID framebuffer);
void wrap_glBindTexture(GLenum target, ID texture);
void wrap_glBufferData(GLenum target, GLsizeiptr size, const GLvoid* data,
                       GLenum usage);
void wrap_glClear(GLbitfield mask);
void wrap_glClearColor(GLclampf red, GLclampf green, GLclampf blue,
                       GLclampf alpha);
void wrap_glCompileShader(ID shader);
PassID wrap_glCreateProgram(void);
PassID wrap_glCreateShader(GLenum type);
void wrap_glDeleteProgram(ID program);
void wrap_glDeleteFramebuffers(GLsizei n, const ID* renderbuffers);
void wrap_glDeleteShader(ID shader);
void wrap_glDeleteTextures(GLsizei n, const ID* textures);
void wrap_glDrawArrays(GLenum mode, GLint first, GLsizei count);
void wrap_glEnableVertexAttribArray(Location index);
void wrap_glFramebufferTexture2D(GLenum target, GLenum attachment,
                                 GLenum textarget, ID texture, GLint level);
void wrap_glGenBuffers(GLsizei n, ID* buffers);
void wrap_glGenFramebuffers(GLsizei n, ID* framebuffers);
void wrap_glGenTextures(GLsizei n, ID* textures);
PassLocation wrap_glGetAttribLocation(ID program, const GLchar* name);
PassLocation wrap_glGetUniformLocation(ID program, const GLchar* name);
void wrap_glLinkProgram(ID program);
void wrap_glShaderSource(ID shader, GLsizei count, const GLchar** string,
                         const GLint* length);
void wrap_glTexImage2D(GLenum target, GLint level, GLint internalformat,
                       GLsizei width, GLsizei height, GLint border,
                       GLenum format, GLenum type, const GLvoid* pixels);
void wrap_glTexParameterf(GLenum target, GLenum pname, GLfloat param);
void wrap_glUniform1f(Location location, GLfloat x);
void wrap_glUniform1i(Location location, GLint x);
void wrap_glUniformMatrix4fv(Location location, GLsizei count,
                             GLboolean transpose, const GLfloat* value);
void wrap_glUseProgram(ID program);
void wrap_glVertexAttribPointer(Location indx, GLint size, GLenum type,
                                GLboolean normalized, GLsizei stride,
                                const GLvoid* ptr);
void wrap_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

}  // namespace gpu

#endif  // GPU_WRAP_GL_H_
