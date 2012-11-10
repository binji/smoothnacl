// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/gl_task.h"
#include <algorithm>
#include <stdio.h>

namespace gpu {

GLTaskList g_task_list;

FunctionGLTask::FunctionGLTask(const char* name, const std::function<FunctionType>& function)
    : name_(name),
      function_(function) {
}

FunctionGLTask::~FunctionGLTask() {
  //printf("Destroying: %s\n", name_);
}

void FunctionGLTask::Run() {
  function_();
}

void GLTaskList::Enqueue(GLTask* task) {
  tasks_.push_back(std::shared_ptr<GLTask>(task));
}

GLTaskList GLTaskList::Take() {
  GLTaskList retval;
  std::swap(retval.tasks_, tasks_);
  return retval;
}

void GLTaskList::RunAndClear() {
  for (Tasks::iterator iter = tasks_.begin(), end = tasks_.end();
       iter != end;
       ++iter) {
    (*iter)->Run();
  }
  tasks_.clear();
}

}  // namespace gpu
