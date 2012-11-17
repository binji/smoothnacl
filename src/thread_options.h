// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THREAD_OPTIONS_H_
#define THREAD_OPTIONS_H_

enum ThreadRunOptions {
  // Run the simulation continuously.
  kRunOptions_Continuous,
  // Wait for a message to step the simulation.
  kRunOptions_Step,
  // Don't run the sim, wait for step message to copy the buffer.
  kRunOptions_None,
};

enum ThreadDrawOptions {
  kDrawOptions_Simulation,
  kDrawOptions_KernelDisc,
  kDrawOptions_KernelRing,
  kDrawOptions_Smoother,
};

#endif  // THREAD_OPTIONS_H_
