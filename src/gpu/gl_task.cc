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
