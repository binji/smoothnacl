#include "smoothlife_thread.h"
#include <algorithm>

SmoothlifeThread::SmoothlifeThread(const ThreadContext& context)
    : context_(context),
      thread_create_result_(0),
      quit_(false) {
  thread_create_result_ = pthread_create(&thread_, NULL, &MainLoopThunk, this);
}

SmoothlifeThread::~SmoothlifeThread() {
  quit_ = true;
  if (thread_create_result_ == 0)
    pthread_join(thread_, NULL);
}

// static
void* SmoothlifeThread::MainLoopThunk(void* param) {
  SmoothlifeThread* self = static_cast<SmoothlifeThread*>(param);
  self->MainLoop();
  return NULL;
}

void SmoothlifeThread::MainLoop() {
  //0 12.0 3.0 12.0 0.100 0.278 0.365 0.267 0.445 4 4 4 0.028 0.147
  //1 31.8 3.0 31.8 0.157 0.092 0.098 0.256 0.607 4 4 4 0.015 0.340
  //1 21.8 3.0 21.8 0.157 0.192 0.200 0.355 0.600 4 4 4 0.025 0.490
  //1 21.8 3.0 21.8 0.157 0.232 0.337 0.599 0.699 4 4 4 0.025 0.290
  //2 12.0 3.0 12.0 0.115 0.269 0.340 0.523 0.746 4 4 4 0.028 0.147
  //2 12.0 3.0 12.0 0.415 0.269 0.350 0.513 0.756 4 4 4 0.028 0.147
  Simulation simulation(context_.config);
  simulation.Clear(0);
  simulation.inita2D(context_.config.kernel_config.ra);

  while (!quit_) {
    AlignedReals* out_data = context_.buffer->Lock();
    std::copy(simulation.buffer().begin(), simulation.buffer().end(),
              out_data->begin());
    context_.buffer->Unlock();

    simulation.Step();
  }
}
