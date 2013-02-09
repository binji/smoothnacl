// Copyright 2013 Ben Smith. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "simulation_thread.h"
#include <algorithm>
#include <time.h>
#include <stdio.h>
#include "draw_strategy_base.h"
#include "initializer_factory_base.h"
#include "screenshot_task.h"
#include "simulation_base.h"
#include "task.h"
#include "worker_thread.h"

namespace {

const int kMinMS = 10;  // 10ms = 100fps
const int kMaxTaskQueueSize = 25;

}  // namespace

SimulationThread::SimulationThread(pp::Instance* instance,
                                   const SimulationThreadContext& context)
    : Thread<SimulationThread>(),
      instance_(instance),
      task_queue_(new ThreadTaskQueue),
      context_(context),
      frames_drawn_(new int),
      simulation_(NULL) {
  last_time_.tv_sec = 0;
  last_time_.tv_usec = 0;
}

void SimulationThread::Destroy() {
  delete simulation_;
  delete draw_strategy_;
}

void SimulationThread::Step() {
  step_cond_.Lock();
  step_cond_.Signal();
  step_cond_.Unlock();
}

void SimulationThread::EnqueueTask(ThreadTask* task) {
  ScopedLocker<ThreadTaskQueue> locker(task_queue_);
  if (locker.object()->size() > kMaxTaskQueueSize) {
    printf("Task queue is full, dropping message.\n");
    return;
  }

  locker.object()->push_back(ThreadTaskPtr(task));
}


int SimulationThread::GetFramesDrawnAndReset() {
  ScopedLocker<int> locker(frames_drawn_);
  int frames_drawn = *locker.object();
  *locker.object() = 0;
  return frames_drawn;
}

void SimulationThread::TaskSetKernel(const KernelConfig& config) {
  simulation_->SetKernel(config);
}

void SimulationThread::TaskSetSmoother(const SmootherConfig& config) {
  simulation_->SetSmoother(config);
}

void SimulationThread::TaskSetPalette(const PaletteConfig& config) {
  draw_strategy_->SetPalette(config);
}

void SimulationThread::TaskClear(double color) {
  simulation_->Clear(color);
}

void SimulationThread::TaskSplat() {
  simulation_->Splat();
}

void SimulationThread::TaskDrawFilledCircle(double x, double y, double radius,
                                            double color) {
  simulation_->DrawFilledCircle(x, y, radius, color);
}

void SimulationThread::TaskSetRunOptions(
    SimulationThreadRunOptions run_options) {
  context_.run_options = run_options;
}

void SimulationThread::TaskSetDrawOptions(
    SimulationThreadDrawOptions draw_options) {
  context_.draw_options = draw_options;
}

void SimulationThread::TaskScreenshot(const ScreenshotConfig& config) {
  AlignedUint32* buffer = draw_strategy_->GetDrawBuffer();
  if (buffer)
    EnqueueWork(new ScreenshotTask(instance_, buffer, config));
}

void SimulationThread::TaskGetBuffer() {
  AlignedReals buffer = simulation_->GetBuffer();
}

// static
void SimulationThread::MainLoop() {
  simulation_ = context_.initializer_factory->CreateSimulation(context_.config);
  draw_strategy_ = context_.initializer_factory->CreateDrawStrategy();

  while (!ShouldQuit()) {
    int* frames = frames_drawn_.Lock();
    (*frames)++;
    frames_drawn_.Unlock();

    // Process queue should be first to allow for any startup initialization.
    ProcessQueue();
    draw_strategy_->Draw(context_.draw_options, simulation_);

    if (context_.run_options & kRunOptions_Simulation)
      simulation_->Step();

    if (context_.run_options & kRunOptions_Pause) {
      // TODO(binji): This is not quite right. The condition may be exited
      // prematurely by another signal firing. What is the correct condition
      // here to loop waiting for?
      step_cond_.Lock();
      step_cond_.Wait();
      step_cond_.Unlock();
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

void SimulationThread::ProcessQueue() {
  ThreadTaskQueue* queue = task_queue_.Lock();
  ThreadTaskQueue copy = *queue;
  queue->clear();
  task_queue_.Unlock();

  ThreadTaskQueue::iterator iter = copy.begin();
  ThreadTaskQueue::iterator end = copy.end();
  for (; iter != end; ++iter)
    (*iter)->Run(this);
}
