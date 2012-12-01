// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIMULATION_THREAD_H_
#define SIMULATION_THREAD_H_

#include <memory>
#include <vector>
#include <sys/time.h>
#include "condvar.h"
#include "locked_object.h"
#include "palette.h"
#include "screenshot_config.h"
#include "simulation_config.h"
#include "simulation_thread_options.h"
#include "task.h"
#include "thread.h"

class DrawStrategyBase;
class InitializerFactoryBase;
class SimulationBase;

class SimulationThread;
typedef Task<SimulationThread> SimulationThreadTask;
typedef std::vector<std::shared_ptr<SimulationThreadTask> >
    SimulationThreadTaskQueue;

struct SimulationThreadContext {
  SimulationConfig config;
  SimulationThreadRunOptions run_options;
  SimulationThreadDrawOptions draw_options;
  InitializerFactoryBase* initializer_factory;  // Weak.
};

class SimulationThread : public Thread<SimulationThread> {
 public:
  typedef Task<SimulationThread> ThreadTask;

  explicit SimulationThread(const SimulationThreadContext& context);

  void Step();
  void EnqueueTask(ThreadTask* task);
  int GetFramesDrawnAndReset();

  // Tasks: Do not call these directly, use MakeFunctionTask(...) instead.
  void TaskSetKernel(const KernelConfig& config);
  void TaskSetSmoother(const SmootherConfig& config);
  void TaskSetPalette(const PaletteConfig& config);
  void TaskClear(double color);
  void TaskSplat();
  void TaskDrawFilledCircle(double x, double y, double radius, double color);
  void TaskSetRunOptions(SimulationThreadRunOptions run_options);
  void TaskSetDrawOptions(SimulationThreadDrawOptions draw_options);
  void TaskScreenshot(const ScreenshotConfig& config);

 protected:
  virtual void Destroy();
  virtual void MainLoop();

 private:
  void ProcessQueue();

  typedef std::shared_ptr<ThreadTask> ThreadTaskPtr;
  typedef std::vector<ThreadTaskPtr> ThreadTaskQueue;

  SimulationThreadContext context_;
  LockedObject<ThreadTaskQueue> task_queue_;
  LockedObject<int> frames_drawn_;
  CondVar step_cond_;
  DrawStrategyBase* draw_strategy_;
  SimulationBase* simulation_;
  struct timeval last_time_;

  SimulationThread(const SimulationThread&);  // Undefined.
  SimulationThread& operator =(const SimulationThread&);  // Undefined.
};

#endif  // SIMULATION_THREAD_H_
