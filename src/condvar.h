#ifndef COND_VAR_H_
#define COND_VAR_H_

#include <pthread.h>
#include "locked_object.h"

class CondVar {
 public:
  CondVar();
  ~CondVar();

  void Lock();
  void Unlock();
  void Broadcast();
  void Signal();
  void Wait();

 private:
  LockedObject<pthread_cond_t> locked_cond_;
  pthread_cond_t* cond_;
};

#endif  // COND_VAR_H_
