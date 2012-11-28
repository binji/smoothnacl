// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIMULATION_THREAD_H_
#define SIMULATION_THREAD_H_

#include <pthread.h>
#include <sys/time.h>
#include "condvar.h"
#include "locked_object.h"
#include "palette.h"
#include "simulation_config.h"
#include "simulation_thread_options.h"
#include "simulation_thread_task_queue.h"

class DrawStrategyBase;
class InitializerFactoryBase;
class SimulationBase;

struct SimulationThreadContext {
  SimulationConfig config;
  SimulationThreadRunOptions run_options;
  SimulationThreadDrawOptions draw_options;
  LockedObject<SimulationThreadTaskQueue>* queue;  // Weak.
  LockedObject<int>* frames_drawn;  // Weak.
  CondVar* step_cond;  // Weak.
  InitializerFactoryBase* initializer_factory;  // Weak.
};

class SimulationThread {
 public:
  explicit SimulationThread(const SimulationThreadContext& context);
  ~SimulationThread();

  // Tasks: Do not call these directly, use MakeFunctionTask(...) instead.
  void TaskSetKernel(const KernelConfig& config);
  void TaskSetSmoother(const SmootherConfig& config);
  void TaskSetPalette(const PaletteConfig& config);
  void TaskClear(double color);
  void TaskSplat();
  void TaskDrawFilledCircle(double x, double y, double radius, double color);
  void TaskSetRunOptions(SimulationThreadRunOptions run_options);
  void TaskSetDrawOptions(SimulationThreadDrawOptions draw_options);
  void TaskScreenshot();

 private:
  static void* MainLoopThunk(void*);
  void MainLoop();
  void ProcessQueue();

  SimulationThreadContext context_;
  DrawStrategyBase* draw_strategy_;
  SimulationBase* simulation_;
  pthread_t thread_;
  int thread_create_result_;
  struct timeval last_time_;
  bool quit_;

  SimulationThread(const SimulationThread&);  // Undefined.
  SimulationThread& operator =(const SimulationThread&);  // Undefined.
};

#endif  // SIMULATION_THREAD_H_
