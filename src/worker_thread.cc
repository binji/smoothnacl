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

#include "worker_thread.h"

namespace {

pthread_once_t initialize_worker_thread_once = PTHREAD_ONCE_INIT;
WorkerThread* g_WorkerThread = NULL;

void InitializeWorkerThread() {
  g_WorkerThread = new WorkerThread;
  g_WorkerThread->Start();
}

}  // namespace

WorkerThread::WorkerThread() {
}

WorkerThread::~WorkerThread() {
}

void WorkerThread::EnqueueTask(ThreadTask* task) {
  not_empty_cond_.Lock();
  task_queue_.push_back(ThreadTaskPtr(task));
  not_empty_cond_.Signal();
  not_empty_cond_.Unlock();
}

void WorkerThread::MainLoop() {
  while (!ShouldQuit()) {
    not_empty_cond_.Lock();
    while (task_queue_.empty()) {
      not_empty_cond_.Wait();
    }

    ThreadTaskPtr task = task_queue_.front();
    task_queue_.pop_front();
    not_empty_cond_.Unlock();

    task->Run(this);
  }
}

void EnqueueWork(WorkerThread::ThreadTask* task) {
  pthread_once(&initialize_worker_thread_once, InitializeWorkerThread);
  g_WorkerThread->EnqueueTask(task);
}
