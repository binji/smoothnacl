// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THREAD_H_
#define THREAD_H_

#include <pthread.h>
#include <sys/time.h>
#include "condvar.h"
#include "locked_object.h"
#include "palette.h"
#include "simulation_config.h"
#include "task_queue.h"
#include "thread_options.h"

class DrawStrategyBase;
class InitializerFactoryBase;
class SimulationBase;

struct ThreadContext {
  SimulationConfig config;
  ThreadRunOptions run_options;
  ThreadDrawOptions draw_options;
  LockedObject<TaskQueue>* queue;  // Weak.
  LockedObject<int>* frames_drawn;  // Weak.
  CondVar* step_cond;  // Weak.
  InitializerFactoryBase* initializer_factory;  // Weak.
};

class Thread {
 public:
  explicit Thread(const ThreadContext& context);
  ~Thread();

  // Tasks: Do not call these directly, use MakeFunctionTask(...) instead.
  void TaskSetKernel(const KernelConfig& config);
  void TaskSetSmoother(const SmootherConfig& config);
  void TaskSetPalette(const PaletteConfig& config);
  void TaskClear(double color);
  void TaskSplat();
  void TaskDrawFilledCircle(double x, double y, double radius, double color);
  void TaskSetRunOptions(ThreadRunOptions run_options);
  void TaskSetDrawOptions(ThreadDrawOptions draw_options);

 private:
  static void* MainLoopThunk(void*);
  void MainLoop();
  void ProcessQueue();

  ThreadContext context_;
  DrawStrategyBase* draw_strategy_;
  SimulationBase* simulation_;
  pthread_t thread_;
  int thread_create_result_;
  struct timeval last_time_;
  bool quit_;

  Thread(const Thread&);  // Undefined.
  Thread& operator =(const Thread&);  // Undefined.
};

#endif  // THREAD_H_
