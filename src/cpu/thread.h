// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CPU_THREAD_H_
#define CPU_THREAD_H_

#include <pthread.h>
#include "condvar.h"
#include "cpu/simulation.h"
#include "fft_allocation.h"
#include "locked_object.h"
#include "task_queue.h"
#include "thread_base.h"
#include "thread_options.h"

namespace cpu {

struct ThreadContext {
  SimulationConfig config;
  ThreadRunOptions run_options;
  ThreadDrawOptions draw_options;
  LockedObject<TaskQueue>* queue;  // Weak.
  LockedObject<AlignedReals>* buffer;  // Weak.
  LockedObject<int>* frames_drawn;  // Weak.
  CondVar* step_cond;  // Weak.
};

class Thread : public ThreadBase {
 public:
  explicit Thread(const ThreadContext& context);
  ~Thread();

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

  Thread(const Thread&);  // Undefined.
  Thread& operator =(const Thread&);  // Undefined.
};

}  // namespace cpu

#endif  // CPU_THREAD_H_
