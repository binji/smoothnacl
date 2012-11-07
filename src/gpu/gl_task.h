// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
  explicit FunctionGLTask(const std::function<FunctionType>& function);
  virtual void Run();

 private:
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
