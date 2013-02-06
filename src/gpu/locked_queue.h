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
