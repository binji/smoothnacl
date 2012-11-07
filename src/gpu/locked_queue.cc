// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/locked_queue.h"

namespace gpu {

LockedQueue::LockedQueue(size_t max_size)
    : max_size_(max_size) {
}

GLTaskList LockedQueue::PopFront() {
  // Consumed by the main thread, so we can't block.
  // Return an empty task list if there is nothing to consume.
  GLTaskList result;
  cond_.Lock();
  if (!container_.empty()) {
    result = container_.front();
    container_.pop_front();
    cond_.Signal();
  }
  cond_.Unlock();
  return result;
}

void LockedQueue::PushBack(const GLTaskList& t) {
  // Produced by the worker thread, so we can block.
  cond_.Lock();
  while (container_.size() == max_size_) {
    cond_.Wait();
  }
  container_.push_back(t);
  cond_.Unlock();
}

}  // namespace gpu
