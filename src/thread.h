// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
    : quit_(new bool),
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
