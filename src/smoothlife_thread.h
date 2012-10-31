// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SMOOTHLIFE_THREAD_H_
#define SMOOTHLIFE_THREAD_H_

#include <pthread.h>
#include "condvar.h"
#include "fft_allocation.h"
#include "locked_object.h"
#include "simulation.h"
#include "task_queue.h"

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

struct ThreadContext {
  SimulationConfig config;
  ThreadRunOptions run_options;
  ThreadDrawOptions draw_options;
  LockedObject<TaskQueue>* queue;  // Weak.
  LockedObject<AlignedReals>* buffer;  // Weak.
  LockedObject<int>* frames_drawn;  // Weak.
  CondVar* step_cond;  // Weak.
};

class SmoothlifeThread {
 public:
  explicit SmoothlifeThread(const ThreadContext& context);
  ~SmoothlifeThread();

  // Tasks: Do not call these directly, use MakeFunctionTask(...) instead.
  void TaskSetKernel(const KernelConfig& config);
  void TaskSetSmoother(const SmootherConfig& config);
  void TaskClear(double color);
  void TaskSplat();
  void TaskDrawFilledCircle(double x, double y, double radius, double color);
  void TaskSetRunOptions(ThreadRunOptions run_options);
  void TaskSetDrawOptions(ThreadDrawOptions draw_options);

 private:
  static void* MainLoopThunk(void*);
  void MainLoop();
  void Draw();
  void CopyBuffer(const AlignedReals& src);
  void ProcessQueue();

  ThreadContext context_;
  Simulation* simulation_;
  pthread_t thread_;
  int thread_create_result_;
  bool quit_;

  SmoothlifeThread(const SmoothlifeThread&);  // Undefined.
  SmoothlifeThread& operator =(const SmoothlifeThread&);  // Undefined.
};

#endif  // SMOOTHLIFE_THREAD_H_
