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
