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

#ifndef GPU_DRAW_STRATEGY_H_
#define GPU_DRAW_STRATEGY_H_

#include <ppapi/cpp/size.h>
#include "draw_strategy_base.h"
#include "gpu/locked_queue.h"
#include "gpu/shader.h"
#include "gpu/vertex_buffer.h"

namespace gpu {

class GLTaskList;

class DrawStrategy : public DrawStrategyBase {
 public:
  DrawStrategy(const pp::Size& size, LockedQueue* locked_queue);
  virtual void Draw(SimulationThreadDrawOptions options,
                    SimulationBase* simulation);

 private:
  void InitShader();
  void Apply(const Texture& in);

  pp::Size size_;
  Shader shader_;
  VertexBuffer vb_;
  LockedQueue* locked_queue_;  // Weak.

  DrawStrategy(const DrawStrategy&);  // Undefined.
  DrawStrategy& operator =(const DrawStrategy&);  // Undefined.
};

}  // namespace gpu

#endif  // GPU_DRAW_STRATEGY_H_
