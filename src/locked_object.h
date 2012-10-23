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

 private:
  T* object_;  // Weak.
  pthread_mutex_t mutex_;
};

#endif  // LOCKED_OBJECT_H_
