// Copyright 2013 Ben Smith. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GPU_TASK_FUNCTIONS_H_
#define GPU_TASK_FUNCTIONS_H_

#include <GLES2/gl2.h>
#include <memory>
#include <stdint.h>
#include "gpu/future.h"

namespace gpu {

typedef std::shared_ptr<uint8_t> UniqueData;
typedef std::shared_ptr<float> UniqueMatrix;
typedef std::shared_ptr<const char*> UniqueStrings;

// Wrapped functions.
void task_glActiveTexture(GLenum texture);
void task_glAttachShader(ID program, ID shader);
void task_glBindBuffer(GLenum target, ID buffer);
void task_glBindFramebuffer(GLenum target, ID framebuffer);
void task_glBindTexture(GLenum target, ID texture);
void task_glBufferData(GLenum target, GLsizeiptr size, UniqueData data,
                       GLenum usage);
void task_glClear(GLbitfield mask);
void task_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void task_glCompileShader(ID shader);
void task_glCreateProgram(ID out_program);
void task_glCreateShader(ID out_shader, GLenum type);
void task_glDeleteProgram(ID program);
void task_glDeleteShader(ID shader);
void task_glDeleteFramebuffer(ID framebuffer);
void task_glDeleteTexture(ID texture);
void task_glDrawArrays(GLenum mode, GLint first, GLsizei count);
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
void task_glShaderSource(ID shader, GLsizei count, UniqueStrings strings,
                         const GLint* length);
void task_glTexImage2D(GLenum target, GLint level, GLint internalformat,
                       GLsizei width, GLsizei height, GLint border,
                       GLenum format, GLenum type, UniqueData pixels);
void task_glTexParameterf(GLenum target, GLenum pname, GLfloat param);
void task_glUniform1f(Location location, GLfloat x);
void task_glUniform2f(Location location, GLfloat x, GLfloat y);
void task_glUniform1i(Location location, GLint x);
void task_glUniformMatrix4fv(Location location, GLsizei count,
                             GLboolean transpose, UniqueMatrix value);
void task_glUseProgram(ID program);
void task_glVertexAttribPointer(Location indx, GLint size, GLenum type,
                                GLboolean normalized, GLsizei stride,
                                const GLvoid* ptr);
void task_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

}  // namespace gpu

#endif  // GPU_TASK_FUNCTIONS_H_
