// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_GL_TASK_H_
#define GPU_GL_TASK_H_

#include <functional>

namespace gpu {

class GLTask;

// Global functions.
void EnqueueTask(GLTask* task);
void ProcessQueue();

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

}  // namespace gpu

#endif  // GPU_GL_TASK_H_
