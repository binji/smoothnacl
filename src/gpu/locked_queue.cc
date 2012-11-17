// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/locked_queue.h"
#include <stdio.h>

#define printf(...)

namespace gpu {

LockedQueue::LockedQueue(size_t max_size)
    : max_size_(max_size) {
}

GLTaskList LockedQueue::PopFront() {
  // Consumed by the main thread, so we can't block.
  // Return an empty task list if there is nothing to consume.
  printf("LockedQueue::PopFront:\n");
  GLTaskList result;
  cond_.Lock();
  if (!container_.empty()) {
    printf("  size:%d -> %d\n", container_.size(), container_.size() - 1);
    result = container_.front();
    container_.pop_front();
    cond_.Signal();
  }
  cond_.Unlock();
  return result;
}

void LockedQueue::PushBack(const GLTaskList& t) {
  // Produced by the worker thread, so we can block.
  printf("LockedQueue::PushBack:\n");
  cond_.Lock();
  while (container_.size() == max_size_) {
    printf("  waiting...\n");
    cond_.Wait();
  }
  printf("  done waiting.\n");
  printf("  size:%d -> %d\n", container_.size(), container_.size() + 1);
  container_.push_back(t);
  cond_.Unlock();
}

}  // namespace gpu
