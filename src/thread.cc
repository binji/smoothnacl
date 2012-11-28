// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "thread.h"
#include <algorithm>
#include <time.h>
#include <stdio.h>
#include "draw_strategy_base.h"
#include "initializer_factory_base.h"
#include "simulation_base.h"
#include "task.h"

namespace {

const int kMinMS = 10;  // 10ms = 100fps

}  // namespace

Thread::Thread(const ThreadContext& context)
    : context_(context),
      simulation_(NULL),
      thread_create_result_(0),
      quit_(false) {
  last_time_.tv_sec = 0;
  last_time_.tv_usec = 0;
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

void Thread::TaskScreenshot() {
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

    struct timeval this_time;
    gettimeofday(&this_time, NULL);
    int diff_ms = (this_time.tv_sec * 1000 + this_time.tv_usec / 1000) -
        (last_time_.tv_sec * 1000 + last_time_.tv_usec / 1000);
    if (diff_ms < kMinMS) {
      struct timespec sleep_time;
      struct timespec rem_time;
      sleep_time.tv_sec = 0;
      sleep_time.tv_nsec = (kMinMS - diff_ms) * 1000000;
      while (nanosleep(&sleep_time, &rem_time) == -1)
        sleep_time = rem_time;
    }

    last_time_ = this_time;
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
