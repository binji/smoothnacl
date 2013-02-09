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

#ifndef POST_BUFFER_TASK_H_
#define POST_BUFFER_TASK_H_

#include <memory>
#include "fft_allocation.h"
#include "task.h"
#include "worker_thread.h"

namespace pp {
class Instance;
class VarArrayBuffer;
}  // namespace pp

class PostBufferTask : public Task<WorkerThread> {
 public:
  PostBufferTask(pp::Instance* instance, AlignedReals* buffer, int request_id);

  virtual void Run(WorkerThread* thread);

 private:
  static void MainThreadRun(void* user_data, int32_t result);

  pp::Instance* instance_;  // Weak.
  std::unique_ptr<AlignedReals> buffer_;
  int request_id_;

  PostBufferTask(const PostBufferTask&);  // Undefined.
  PostBufferTask& operator =(const PostBufferTask&);  // Undefined.
};

#endif  // POST_BUFFER_TASK_H_
