#ifndef SMOOTHLIFE_THREAD_H_
#define SMOOTHLIFE_THREAD_H_

#include <pthread.h>
#include "fft_allocation.h"
#include "locked_object.h"
#include "simulation.h"
#include "task_queue.h"

struct ThreadContext {
  SimulationConfig config;
  LockedObject<TaskQueue>* queue;  // Weak.
  LockedObject<AlignedReals>* buffer;  // Weak.
  LockedObject<int>* frames_drawn;  // Weak.
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

 private:
  static void* MainLoopThunk(void*);
  void MainLoop();
  void CopyBuffer();
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
