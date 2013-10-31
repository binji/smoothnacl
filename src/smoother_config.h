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

#ifndef SMOOTHER_CONFIG_H_
#define SMOOTHER_CONFIG_H_

enum Timestep {
  TIMESTEP_DISCRETE,
  TIMESTEP_SMOOTH1,
  TIMESTEP_SMOOTH2,
  TIMESTEP_SMOOTH3,
  TIMESTEP_SMOOTH4
};

struct TimestepConfig {
  Timestep type;
  double dt;
};

enum Sigmoid {
  SIGMOID_HARD,
  SIGMOID_LINEAR,
  SIGMOID_HERMITE,
  SIGMOID_SIN,
  SIGMOID_SMOOTH
};

enum SigmoidMode {
  // mix(sigmoid_ab(n, b1, b2), sigmoid_ab(n, d1, d2), m)
  SIGMOID_MODE_1,
  // sigmoid_mix(sigmoid_ab(n, b1, b2), sigmoid_ab(n, d1, d2), m)
  SIGMOID_MODE_2,
  // sigmoid_ab(n, mix(b1, d1, m), mix(b2, d2, m))
  SIGMOID_MODE_3,
  // sigmoid_ab(n, sigmoid_mix(b1, d1, m), sigmoid_mix(b2, d2, m))
  SIGMOID_MODE_4
};

struct SmootherConfig {
  SmootherConfig() :
    b1(0), d1(0), b2(0), d2(0),
    mode(SIGMOID_MODE_1), sigmoid(SIGMOID_HARD), mix(SIGMOID_HARD),
    sn(0), sm(0) {}

  TimestepConfig timestep;
  double b1;
  double d1;
  double b2;
  double d2;
  SigmoidMode mode;
  Sigmoid sigmoid;
  Sigmoid mix;
  double sn;
  double sm;
};

#endif  // SMOOTHER_CONFIG_H_
