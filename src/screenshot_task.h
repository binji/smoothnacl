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
#include "task.h"
#include "worker_thread.h"

namespace pp {
class Instance;
class VarArrayBuffer;
}  // namespace pp

struct ImageDeleter {
  void operator ()(imImage* image) const;
};
typedef std::unique_ptr<imImage, ImageDeleter> ImagePtr;

struct FileDeleter {
  void operator ()(imFile* file) const;
};
typedef std::unique_ptr<imFile, FileDeleter> FilePtr;

struct MemoryFilenameDeleter {
  void operator ()(imBinMemoryFileName* filename) const;
};
typedef std::unique_ptr<imBinMemoryFileName, MemoryFilenameDeleter>
    MemoryFilenamePtr;

class ImageOperation {
 public:
  virtual ~ImageOperation() {}
  virtual ImagePtr Run(ImagePtr src) = 0;
};

class ReduceImageOperation : public ImageOperation {
 public:
  explicit ReduceImageOperation(size_t max_length);
  virtual ImagePtr Run(ImagePtr src);

 private:
  size_t max_length_;
};

class CropImageOperation : public ImageOperation {
 public:
  CropImageOperation(double x_scale, double y_scale, size_t max_length);
  virtual ImagePtr Run(ImagePtr src);

 private:
  double x_scale_;
  double y_scale_;
  size_t max_length_;
};

class BrightnessContrastImageOperation : public ImageOperation {
 public:
  BrightnessContrastImageOperation(double brightness_shift,
                                   double contrast_factor);
  virtual ImagePtr Run(ImagePtr src);

 private:
  double brightness_shift_;
  double contrast_factor_;
};

class ScreenshotConfig {
 public:
  ScreenshotConfig();
  ~ScreenshotConfig();

  std::vector<ImageOperation*> operations;
};

class ScreenshotTask : public Task<WorkerThread> {
 public:
  ScreenshotTask(pp::Instance* instance, AlignedUint32* buffer);

  virtual void Run(WorkerThread* thread);

 private:
  ImagePtr BufferToImage();
  FilePtr WriteImageToFile(imBinMemoryFileName* filename, ImagePtr image);
  pp::VarArrayBuffer FileToArrayBuffer(imBinMemoryFileName* filename,
                                       FilePtr file);
  static void MainThreadRun(void* user_data, int32_t result);

  pp::Instance* instance_;  // Weak.
  std::unique_ptr<AlignedUint32> buffer_;

  ScreenshotTask(const ScreenshotTask&);  // Undefined.
  ScreenshotTask& operator =(const ScreenshotTask&);  // Undefined.
};

#endif  // SCREENSHOT_TASK_H_
