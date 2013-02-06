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

#ifndef GPU_SHADER_H_
#define GPU_SHADER_H_

#include <map>
#include <string>
#include "gpu/wrap_gl.h"

namespace gpu {

class Texture;

class Shader {
 public:
  Shader();
  ~Shader();

  void Init(const char* frag_shader, const char* vertex_shader);

  Location GetAttribLocation(const char* name);
  Location GetUniformLocation(const char* name);
  void Use();
  void Uniform1f(const char* name, GLfloat value);
  void Uniform2f(const char* name, GLfloat value0, GLfloat value1);
  void Uniform1i(const char* name, GLint value);
  void UniformTexture(const char* name, int index, const Texture& texture);
  void UniformMatrixOrtho(const char* name, float l, float r, float b, float t,
                          float near, float far);

 private:
  PassID CompileShader(GLenum type, const char* data);

  ID id_;
  ID vert_id_;
  ID frag_id_;
  typedef std::map<std::string, Location> NameLocationMap;
  NameLocationMap attribs_;
  NameLocationMap uniforms_;

  Shader(const Shader&);  // undefined
  Shader& operator =(const Shader&);  // undefined
};

}  // namespace gpu

#endif  // GPU_SHADER_H_
