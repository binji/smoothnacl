// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_SMOOTHER_H_
#define GPU_SMOOTHER_H_

#include <ppapi/cpp/size.h>
#include "smoother_config.h"
#include "gpu/shader.h"
#include "gpu/vertex_buffer.h"

namespace gpu {

class Texture;

class Smoother {
 public:
  Smoother(const pp::Size& size, const SmootherConfig& config);
  ~Smoother();

  const pp::Size& size() const { return size_; }
  const SmootherConfig& config() const { return config_; }
  void SetConfig(const SmootherConfig& config);

  void Apply(const Texture& in0, const Texture& in1, Texture& out);

 private:
  pp::Size size_;
  SmootherConfig config_;
  Shader shader_;
  VertexBuffer vb_;

  Smoother(const Smoother&);  // Undefined.
  Smoother& operator =(const Smoother&);  // Undefined.
};

}  // namespace gpu

#endif  // GPU_SMOOTHER_H_
