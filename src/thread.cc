// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "thread.h"
#include <algorithm>
#include "draw_strategy_base.h"
#include "initializer_factory_base.h"
#include "simulation_base.h"
#include "task.h"

Thread::Thread(const ThreadContext& context)
    : context_(context),
      simulation_(NULL),
      thread_create_result_(0),
      quit_(false) {
  thread_create_result_ = pthread_create(&thread_, NULL, &MainLoopThunk, this);
}

Thread::~Thread() {
  quit_ = true;
  if (thread_create_result_ == 0)
    pthread_join(thread_, NULL);
}

void Thread::TaskSetKernel(const KernelConfig& config) {
  simulation_->SetKernel(config);
}

void Thread::TaskSetSmoother(const SmootherConfig& config) {
  simulation_->SetSmoother(config);
}

void Thread::TaskSetPalette(const PaletteConfig& config) {
  draw_strategy_->SetPalette(config);
}

void Thread::TaskClear(double color) {
  simulation_->Clear(color);
}

void Thread::TaskSplat() {
  simulation_->Splat();
}

void Thread::TaskDrawFilledCircle(double x, double y, double radius,
                                            double color) {
  simulation_->DrawFilledCircle(x, y, radius, color);
}

void Thread::TaskSetRunOptions(ThreadRunOptions run_options) {
  context_.run_options = run_options;
}

void Thread::TaskSetDrawOptions(ThreadDrawOptions draw_options) {
  context_.draw_options = draw_options;
}

// static
void* Thread::MainLoopThunk(void* param) {
  Thread* self = static_cast<Thread*>(param);
  self->MainLoop();
  return NULL;
}

void Thread::MainLoop() {
  simulation_ = context_.initializer_factory->CreateSimulation(context_.config);
  draw_strategy_ = context_.initializer_factory->CreateDrawStrategy();

  while (!quit_) {
    int* frames = context_.frames_drawn->Lock();
    (*frames)++;
    context_.frames_drawn->Unlock();

    // Process queue should be first to allow for any startup initialization.
    ProcessQueue();
    draw_strategy_->Draw(context_.draw_options, simulation_);

    if (context_.run_options & kRunOptions_Simulation)
      simulation_->Step();

    if (context_.run_options & kRunOptions_Pause) {
      // TODO(binji): This is not quite right. The condition may be exited
      // prematurely by another signal firing. What is the correct condition
      // here to loop waiting for?
      context_.step_cond->Lock();
      context_.step_cond->Wait();
      context_.step_cond->Unlock();
    }
  }
}

void Thread::ProcessQueue() {
  TaskQueue* queue = context_.queue->Lock();
  TaskQueue copy = *queue;
  queue->clear();
  context_.queue->Unlock();

  for (TaskQueue::iterator iter = copy.begin(), end = copy.end();
       iter != end;
       ++iter) {
    (*iter)->Run(this);
  }
}
