// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
