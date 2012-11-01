// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIMULATION_CONFIG_H_
#define SIMULATION_CONFIG_H_

struct SimulationConfig {
  pp::Size size;
  KernelConfig kernel_config;
  SmootherConfig smoother_config;
};

#endif  // SIMULATION_CONFIG_H_
