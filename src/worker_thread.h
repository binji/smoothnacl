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

#ifndef WORKER_THREAD_H_
#define WORKER_THREAD_H_

#include <deque>
#include <memory>
#include "condvar.h"
#include "task.h"
#include "thread.h"

class WorkerThread : public Thread<WorkerThread> {
 public:
  typedef Task<WorkerThread> ThreadTask;

  WorkerThread();
  ~WorkerThread();

  void EnqueueTask(ThreadTask* task);

 protected:
  virtual void MainLoop();

 private:
  typedef std::shared_ptr<ThreadTask> ThreadTaskPtr;
  typedef std::deque<ThreadTaskPtr> ThreadTaskQueue;

  CondVar not_empty_cond_;
  ThreadTaskQueue task_queue_;

  WorkerThread(const WorkerThread&);  // Undefined.
  WorkerThread& operator =(const WorkerThread&);  // Undefined.
};

void EnqueueWork(WorkerThread::ThreadTask* task);

#endif  // WORKER_THREAD_H_
