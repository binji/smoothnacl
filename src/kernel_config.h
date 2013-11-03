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

#ifndef KERNEL_CONFIG_H_
#define KERNEL_CONFIG_H_

struct KernelConfig {
  KernelConfig() : disc_radius(0), ring_radius(0), blend_radius(0) {}

  real disc_radius;
  real ring_radius;
  real blend_radius;
};

#endif  // KERNEL_CONFIG_H_
