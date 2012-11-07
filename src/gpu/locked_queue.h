// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_LOCKED_QUEUE_H_
#define GPU_LOCKED_QUEUE_H_

#include <assert.h>
#include <deque>
#include "condvar.h"
#include "gpu/gl_task.h"

namespace gpu {

class LockedQueue {
 public:
  explicit LockedQueue(size_t max_size);

  GLTaskList PopFront();
  void PushBack(const GLTaskList& t);

 private:
  std::deque<GLTaskList> container_;
  size_t max_size_;
  CondVar cond_;
};

}  // namespace gpu

#endif  // GPU_LOCKED_QUEUE_H_
