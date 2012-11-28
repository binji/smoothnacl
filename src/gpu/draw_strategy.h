// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
