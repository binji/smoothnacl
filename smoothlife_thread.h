#ifndef SMOOTHLIFE_THREAD_H_
#define SMOOTHLIFE_THREAD_H_

#include <pthread.h>
#include "fft_allocation.h"
#include "locked_object.h"
#include "simulation.h"
//#include "task_queue.h"

struct ThreadContext {
  SimulationConfig config;
  //TaskQueue queue;
  LockedObject<AlignedReals>* buffer;  // Weak.
};

class SmoothlifeThread {
 public:
  explicit SmoothlifeThread(const ThreadContext& context);
  ~SmoothlifeThread();

 private:
  static void* MainLoopThunk(void*);
  void MainLoop();

  ThreadContext context_;
  pthread_t thread_;
  int thread_create_result_;
  bool quit_;

  SmoothlifeThread(const SmoothlifeThread&);  // Undefined.
  SmoothlifeThread& operator =(const SmoothlifeThread&);  // Undefined.
};

#endif  // SMOOTHLIFE_THREAD_H_
