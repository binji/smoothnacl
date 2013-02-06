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

#ifndef GPU_DRAW_CIRCLE_H_
#define GPU_DRAW_CIRCLE_H_

#include <ppapi/cpp/size.h>
#include <vector>
#include "gpu/shader.h"
#include "gpu/vertex_buffer.h"

namespace gpu {

class Texture;

struct Circle {
  float x;
  float y;
  float radius;
};
typedef std::vector<Circle> Circles;

class DrawCircle {
 public:
  explicit DrawCircle(const pp::Size& size);
  ~DrawCircle();

  void Apply(Texture& inout, float x, float y, float radius);
  void Apply(Texture& inout, const Circles& circles);

 private:
  pp::Size size_;
  Shader shader_;
  VertexBuffer vb_;

  DrawCircle(const DrawCircle&);  // Undefined.
  DrawCircle& operator =(const DrawCircle&);  // Undefined.
};

}  // namespace gpu

#endif  // GPU_DRAW_CIRCLE_H_

