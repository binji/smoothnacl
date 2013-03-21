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

#ifndef SIMULATION_THREAD_H_
#define SIMULATION_THREAD_H_

#include <memory>
#include <vector>
#include <sys/time.h>
#include "condvar.h"
#include "locked_object.h"
#include "palette.h"
#include "simulation_config.h"
#include "simulation_thread_options.h"
#include "task.h"
#include "thread.h"

namespace pp {
class Instance;
}  // namespace pp

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

  SimulationThread(pp::Instance* instance_,
                   const SimulationThreadContext& context);

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
  void TaskGetBuffer(int request_id);

 protected:
  virtual void Destroy();
  virtual void MainLoop();

 private:
  void ProcessQueue();

  typedef std::shared_ptr<ThreadTask> ThreadTaskPtr;
  typedef std::vector<ThreadTaskPtr> ThreadTaskQueue;

  pp::Instance* instance_;
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
