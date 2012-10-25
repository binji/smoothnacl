#include "condvar.h"

namespace {

pthread_cond_t* InitializeNewCond() {
  pthread_cond_t* cond = new pthread_cond_t;
  pthread_cond_init(cond, NULL);
  return cond;
}

}  // namespace

CondVar::CondVar()
    : locked_cond_(InitializeNewCond()),
      cond_(NULL) {
}

CondVar::~CondVar() {
  ScopedLocker<pthread_cond_t> locker(locked_cond_);
  cond_ = locker.object();

  pthread_cond_destroy(cond_);
  delete cond_;
}

void CondVar::Lock() {
  cond_ = locked_cond_.Lock();
}

void CondVar::Unlock() {
  cond_ = NULL;
  locked_cond_.Unlock();
}

void CondVar::Broadcast() {
  pthread_cond_broadcast(cond_);
}

void CondVar::Signal() {
  pthread_cond_signal(cond_);
}

void CondVar::Wait() {
  pthread_cond_wait(cond_, locked_cond_.mutex());
}
