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

#ifndef GPU_REAL_TO_COMPLEX_H_
#define GPU_REAL_TO_COMPLEX_H_

#include <ppapi/cpp/size.h>
#include "gpu/shader.h"
#include "gpu/vertex_buffer.h"

namespace gpu {

class Texture;

class RealToComplex {
 public:
  explicit RealToComplex(const pp::Size& size);
  ~RealToComplex();

  void Apply(const Texture& in, Texture& out);

 private:
  pp::Size size_;
  Shader shader_;
  VertexBuffer vb_;

  RealToComplex(const RealToComplex&);  // Undefined.
  RealToComplex& operator =(const RealToComplex&);  // Undefined.
};

}  // namespace gpu

#endif  // GPU_REAL_TO_COMPLEX_H_
