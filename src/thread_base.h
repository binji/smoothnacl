// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THREAD_BASE_H_
#define THREAD_BASE_H_

#include "thread_options.h"

class KernelConfig;
class SmootherConfig;

class ThreadBase {
 public:
  // Tasks: Do not call these directly, use MakeFunctionTask(...) instead.
  virtual void TaskSetKernel(const KernelConfig& config) = 0;
  virtual void TaskSetSmoother(const SmootherConfig& config) = 0;
  virtual void TaskClear(double color) = 0;
  virtual void TaskSplat() = 0;
  virtual void TaskDrawFilledCircle(double x, double y, double radius, double color) = 0;
  virtual void TaskSetRunOptions(ThreadRunOptions run_options) = 0;
  virtual void TaskSetDrawOptions(ThreadDrawOptions draw_options) = 0;
};

#endif  // THREAD_BASE_H_
