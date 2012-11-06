// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_WRAP_GL_H_
#define GPU_WRAP_GL_H_

#include <GLES2/gl2.h>
#include "gpu/future.h"

namespace gpu {

void glActiveTexture(GLenum texture);
void glAttachShader(ID program, ID shader);
void glBindBuffer(GLenum target, ID buffer);
void glBindFramebuffer(GLenum target, ID framebuffer);
void glBindTexture(GLenum target, ID texture);
void glBufferData(GLenum target, GLsizeiptr size, const GLvoid* data,
                  GLenum usage);
void glClear(GLbitfield mask);
void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void glCompileShader(ID shader);
ID glCreateProgram(void);
ID glCreateShader(GLenum type);
void glDeleteProgram(ID program);
void glDeleteFramebuffers(GLsizei n, const ID* renderbuffers);
void glDeleteShader(ID shader);
void glDeleteTextures(GLsizei n, const ID* textures);
void glDrawArrays(GLenum mode, GLint first, GLsizei count);
void glEnableVertexAttribArray(Location index);
void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget,
                            ID texture, GLint level);
void glGenBuffers(GLsizei n, ID* buffers);
void glGenFramebuffers(GLsizei n, ID* framebuffers);
void glGenTextures(GLsizei n, ID* textures);
Location glGetAttribLocation(ID program, const GLchar* name);
Location glGetUniformLocation(ID program, const GLchar* name);
void glLinkProgram(ID program);
void glShaderSource(ID shader, GLsizei count, const GLchar** string,
                    const GLint* length);
void glTexImage2D(GLenum target, GLint level, GLint internalformat,
                  GLsizei width, GLsizei height, GLint border, GLenum format,
                  GLenum type, const GLvoid* pixels);
void glTexParameterf(GLenum target, GLenum pname, GLfloat param);
void glUniform1f(Location location, GLfloat x);
void glUniform1i(Location location, GLint x);
void glUniformMatrix4fv(Location location, GLsizei count, GLboolean transpose,
                        const GLfloat* value);
void glUseProgram(ID program);
void glVertexAttribPointer(Location indx, GLint size, GLenum type,
                           GLboolean normalized, GLsizei stride,
                           const GLvoid* ptr);
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

}  // namespace gpu

#endif  // GPU_WRAP_GL_H_
