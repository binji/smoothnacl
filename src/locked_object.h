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

#ifndef LOCKED_OBJECT_H_
#define LOCKED_OBJECT_H_

#include <assert.h>
#include <pthread.h>

template<typename T>
class LockedObject {
 public:
  explicit LockedObject(T* object)
      : object_(object) {
    pthread_mutex_init(&mutex_, NULL);
  }

  ~LockedObject() {
    pthread_mutex_destroy(&mutex_);
  }

  T* Lock() {
    int result = pthread_mutex_lock(&mutex_);
    assert(result == 0);
    return object_;
  }

  void Unlock() {
    pthread_mutex_unlock(&mutex_);
  }

  pthread_mutex_t* mutex() { return &mutex_; }

 private:
  T* object_;  // Weak.
  pthread_mutex_t mutex_;
};

template<typename T>
class ScopedLocker {
 public:
  explicit ScopedLocker(LockedObject<T>& lock)
      : lock_(lock) {
    obj_ = lock.Lock();
  }

  ~ScopedLocker() {
    lock_.Unlock();
  }

  T* object() { return obj_; }

 private:
  LockedObject<T>& lock_;
  T* obj_;

  ScopedLocker(const ScopedLocker&);  // Undefined.
  ScopedLocker& operator =(const ScopedLocker&);  // Undefined.
};

#endif  // LOCKED_OBJECT_H_
