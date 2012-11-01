// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
  // sigmoid_ab(n, mix(b1, d1, m), mix(b1, b2, m))
  SIGMOID_MODE_3,
  // sigmoid_ab(n, sigmoid_mix(b1, d1, m), sigmoid_mix(b1, b2, m))
  SIGMOID_MODE_4
};

struct SmootherConfig {
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
