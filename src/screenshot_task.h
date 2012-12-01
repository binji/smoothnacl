// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SCREENSHOT_TASK_H_
#define SCREENSHOT_TASK_H_

#include <im.h>
#include <im_binfile.h>
#include <im_image.h>
#include <memory>
#include <vector>
#include "fft_allocation.h"
#include "image_operation.h"
#include "screenshot_config.h"
#include "task.h"
#include "worker_thread.h"

namespace pp {
class Instance;
class VarArrayBuffer;
}  // namespace pp

class ScreenshotTask : public Task<WorkerThread> {
 public:
  ScreenshotTask(pp::Instance* instance, AlignedUint32* buffer,
                 const ScreenshotConfig& config);

  virtual void Run(WorkerThread* thread);

 private:
  ImagePtr BufferToImage();
  FilePtr WriteImageToFile(imBinMemoryFileName* filename, ImagePtr image);
  pp::VarArrayBuffer FileToArrayBuffer(imBinMemoryFileName* filename,
                                       FilePtr file);
  static void MainThreadRun(void* user_data, int32_t result);

  pp::Instance* instance_;  // Weak.
  std::unique_ptr<AlignedUint32> buffer_;
  ScreenshotConfig config_;

  ScreenshotTask(const ScreenshotTask&);  // Undefined.
  ScreenshotTask& operator =(const ScreenshotTask&);  // Undefined.
};

#endif  // SCREENSHOT_TASK_H_
