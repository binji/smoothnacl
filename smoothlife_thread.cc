#include "smoothlife_thread.h"
#include <algorithm>
#include "task.h"

SmoothlifeThread::SmoothlifeThread(const ThreadContext& context)
    : context_(context),
      simulation_(NULL),
      thread_create_result_(0),
      quit_(false) {
  thread_create_result_ = pthread_create(&thread_, NULL, &MainLoopThunk, this);
}

SmoothlifeThread::~SmoothlifeThread() {
  quit_ = true;
  if (thread_create_result_ == 0)
    pthread_join(thread_, NULL);
}

void SmoothlifeThread::TaskSetKernel(const KernelConfig& config) {
  simulation_->SetKernel(config);
}

void SmoothlifeThread::TaskSetSmoother(const SmootherConfig& config) {
  simulation_->SetSmoother(config);
}

void SmoothlifeThread::TaskClear(double color) {
  simulation_->Clear(color);
}

void SmoothlifeThread::TaskSplat() {
  simulation_->Splat();
}

void SmoothlifeThread::TaskDrawFilledCircle(double x, double y, double radius,
                                            double color) {
  simulation_->DrawFilledCircle(x, y, radius, color);
}

// static
void* SmoothlifeThread::MainLoopThunk(void* param) {
  SmoothlifeThread* self = static_cast<SmoothlifeThread*>(param);
  self->MainLoop();
  return NULL;
}

void SmoothlifeThread::MainLoop() {
  simulation_ = new Simulation(context_.config);
  simulation_->Clear(0);
  simulation_->Splat();

  while (!quit_) {
    CopyBuffer();
    ProcessQueue();
    simulation_->Step();
  }
}

void SmoothlifeThread::CopyBuffer() {
  AlignedReals* out_data = context_.buffer->Lock();
  std::copy(simulation_->buffer().begin(), simulation_->buffer().end(),
            out_data->begin());
  context_.buffer->Unlock();
}

void SmoothlifeThread::ProcessQueue() {
  TaskQueue* queue = context_.queue->Lock();
  for (TaskQueue::iterator iter = queue->begin(), end = queue->end();
       iter != end;
       ++iter) {
    (*iter)->Run(this);
  }
  queue->clear();
  context_.queue->Unlock();
}
