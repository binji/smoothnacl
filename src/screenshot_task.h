// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SCREENSHOT_TASK_H_
#define SCREENSHOT_TASK_H_

#include <im.h>
#include <im_binfile.h>
#include "fft_allocation.h"
#include "task.h"
#include "worker_thread.h"

namespace pp {
class Instance;
}  // namespace pp

class ScreenshotTask : public Task<WorkerThread> {
 public:
  ScreenshotTask(pp::Instance* instance, AlignedUint32* buffer);
  ~ScreenshotTask();

  virtual void Run(WorkerThread* thread);

 private:
  static void MainThreadRun(void* user_data, int32_t result);

  pp::Instance* instance_;  // Weak.
  AlignedUint32* buffer_;
  imFile* file_;
  imBinMemoryFileName* filename_;

  ScreenshotTask(const ScreenshotTask&);  // Undefined.
  ScreenshotTask& operator =(const ScreenshotTask&);  // Undefined.
};

#endif  // SCREENSHOT_TASK_H_
