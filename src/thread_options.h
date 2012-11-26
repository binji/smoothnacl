// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THREAD_OPTIONS_H_
#define THREAD_OPTIONS_H_

enum {
  kRunOptions_Simulation = 1,
  kRunOptions_Pause = 2,
};

typedef int ThreadRunOptions;

enum ThreadDrawOptions {
  kDrawOptions_Simulation,
  kDrawOptions_KernelDisc,
  kDrawOptions_KernelRing,
  kDrawOptions_Smoother,
  kDrawOptions_Palette,
};

#endif  // THREAD_OPTIONS_H_
