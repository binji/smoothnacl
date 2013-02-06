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

#ifndef GPU_KERNEL_MUL_H_
#define GPU_KERNEL_MUL_H_

#include <ppapi/cpp/size.h>
#include "gpu/shader.h"
#include "gpu/vertex_buffer.h"

namespace gpu {

class Texture;

class KernelMul {
 public:
  explicit KernelMul(const pp::Size& size);
  ~KernelMul();

  void Apply(const Texture& in0, const Texture& in1, Texture& out,
             double scale);

 private:
  pp::Size size_;
  Shader shader_;
  VertexBuffer vb_;

  KernelMul(const KernelMul&);  // Undefined.
  KernelMul& operator =(const KernelMul&);  // Undefined.
};

}  // namespace gpu

#endif  // GPU_KERNEL_MUL_H_
