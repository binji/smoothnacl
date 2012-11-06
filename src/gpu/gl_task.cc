// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/gl_task.h"
#include <memory>
#include <vector>

namespace gpu {

namespace {

typedef std::vector<std::shared_ptr<GLTask> > TaskQueue;
TaskQueue g_tasks;

}  // namespace

void EnqueueTask(GLTask* task) {
  g_tasks.push_back(std::shared_ptr<GLTask>(task));
}

void ProcessQueue() {
  for (TaskQueue::iterator iter = g_tasks.begin(), end = g_tasks.end();
       iter != end;
       ++iter) {
    (*iter)->Run();
  }
  g_tasks.clear();
}

FunctionGLTask::FunctionGLTask(const std::function<FunctionType>& function)
    : function_(function) {
}

void FunctionGLTask::Run() {
  function_();
}

}  // namespace gpu
