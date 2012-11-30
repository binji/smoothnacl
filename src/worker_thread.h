// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
