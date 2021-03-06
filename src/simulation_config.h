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

#ifndef SIMULATION_CONFIG_H_
#define SIMULATION_CONFIG_H_

#include <ppapi/cpp/size.h>
#include "kernel_config.h"
#include "smoother_config.h"

struct SimulationConfig {
  explicit SimulationConfig(int thread_count, const pp::Size& size)
      : thread_count(thread_count),
        size(size) {}
  int thread_count;
  pp::Size size;
  KernelConfig kernel_config;
  SmootherConfig smoother_config;
};

#endif  // SIMULATION_CONFIG_H_
