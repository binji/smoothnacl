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

#ifndef THREAD_H_
#define THREAD_H_

#include <pthread.h>
#include <stdio.h>
#include "locked_object.h"

template <typename T>
class Thread {
 public:
  Thread();
  virtual ~Thread();

  void Start();

 protected:
  virtual void Destroy() {}
  virtual void MainLoop() = 0;
  bool ShouldQuit();

 private:
  static void* MainLoopThunk(void*);

  LockedObject<bool> quit_;
  pthread_t thread_;
  int thread_create_result_;

  Thread(const Thread&);  // Undefined.
  Thread& operator =(const Thread&);  // Undefined.
};


template <typename T>
Thread<T>::Thread()
    : quit_(new bool(false)),
      thread_create_result_(0) {
}

template <typename T>
Thread<T>::~Thread() {
  bool* quit = quit_.Lock();
  *quit = true;
  quit_.Unlock();

  if (thread_create_result_ == 0)
    pthread_join(thread_, NULL);

  Destroy();
}

template <typename T>
void Thread<T>::Start() {
  thread_create_result_ = pthread_create(&thread_, NULL, &MainLoopThunk, this);
}

template <typename T>
bool Thread<T>::ShouldQuit() {
  ScopedLocker<bool> locker(quit_);
  return *locker.object();
}

// static
template <typename T>
void* Thread<T>::MainLoopThunk(void* param) {
  Thread<T>* self = static_cast<Thread<T>*>(param);
  self->MainLoop();
  return NULL;
}

#endif  // THREAD_H_
