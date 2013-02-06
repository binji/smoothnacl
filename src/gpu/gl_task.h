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

#ifndef GPU_GL_TASK_H_
#define GPU_GL_TASK_H_

#include <functional>
#include <memory>
#include <vector>

namespace gpu {

class GLTask {
 public:
  virtual ~GLTask() {}
  virtual void Run() = 0;
};

class FunctionGLTask : public GLTask {
 public:
  typedef void FunctionType();
  explicit FunctionGLTask(const char* name, const std::function<FunctionType>& function);
  ~FunctionGLTask();
  virtual void Run();

 private:
  const char* name_;
  std::function<FunctionType> function_;
};

class GLTaskList {
 public:
  void Enqueue(GLTask* task);
  GLTaskList Take();
  void RunAndClear();

 private:
  typedef std::vector<std::shared_ptr<GLTask> > Tasks;

  Tasks tasks_;
};

extern GLTaskList g_task_list;

}  // namespace gpu

#endif  // GPU_GL_TASK_H_
