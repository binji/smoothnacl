// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIMULATION_THREAD_OPTIONS_H_
#define SIMULATION_THREAD_OPTIONS_H_

enum {
  kRunOptions_Simulation = 1,
  kRunOptions_Pause = 2,
};

typedef int SimulationThreadRunOptions;

enum SimulationThreadDrawOptions {
  kDrawOptions_Simulation,
  kDrawOptions_KernelDisc,
  kDrawOptions_KernelRing,
  kDrawOptions_Smoother,
  kDrawOptions_Palette,
};

#endif  // SIMULATION_THREAD_OPTIONS_H_
