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
                                       uint32_t request_id,
                                       FilePtr file);
  static void MainThreadRun(void* user_data, int32_t result);

  pp::Instance* instance_;  // Weak.
  std::unique_ptr<AlignedUint32> buffer_;
  ScreenshotConfig config_;

  ScreenshotTask(const ScreenshotTask&);  // Undefined.
  ScreenshotTask& operator =(const ScreenshotTask&);  // Undefined.
};

#endif  // SCREENSHOT_TASK_H_
