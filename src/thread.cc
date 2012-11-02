// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "thread.h"
#include <algorithm>
#include "draw_strategy_base.h"
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
  simulation_ = context_.factory->Create(context_.config);
  while (!quit_) {
    int* frames = context_.frames_drawn->Lock();
    (*frames)++;
    context_.frames_drawn->Unlock();

    // Process queue should be first to allow for any startup initialization.
    ProcessQueue();
    context_.draw_strategy->Draw(context_.draw_options, simulation_);

    if (context_.run_options != kRunOptions_None)
      simulation_->Step();

    if (context_.run_options != kRunOptions_Continuous) {
      context_.step_cond->Lock();
      context_.step_cond->Wait();
      context_.step_cond->Unlock();
    }
  }
}

void Thread::ProcessQueue() {
  TaskQueue* queue = context_.queue->Lock();
  for (TaskQueue::iterator iter = queue->begin(), end = queue->end();
       iter != end;
       ++iter) {
    (*iter)->Run(this);
  }
  queue->clear();
  context_.queue->Unlock();
}
