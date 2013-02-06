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

#include "screenshot_task.h"
#include <algorithm>
#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/var_array_buffer.h>
#include <stdio.h>


namespace {

struct PostMessageData {
  PostMessageData() : instance(NULL), buffer(0) {}
  pp::Instance* instance;
  pp::VarArrayBuffer buffer;
};

}  // namespace

ScreenshotTask::ScreenshotTask(pp::Instance* instance, AlignedUint32* buffer,
                               const ScreenshotConfig& config)
    : instance_(instance),
      buffer_(buffer),
      config_(config) {
}

void ScreenshotTask::Run(WorkerThread*) {
  ImagePtr image = BufferToImage();
  ScreenshotConfig::Operations::iterator iter = config_.operations.begin();
  for (; iter != config_.operations.end(); ++iter)
    image = (*iter)->Run(std::move(image));

  MemoryFilenamePtr filename(new imBinMemoryFileName);
  filename->buffer = NULL;
  filename->size = 10 * 1024;
  filename->reallocate = 1.5f;

  FilePtr file = WriteImageToFile(filename.get(), std::move(image));
  pp::VarArrayBuffer array_buffer = FileToArrayBuffer(filename.get(),
                                                      config_.request_id,
                                                      std::move(file));

  PostMessageData* post_message_data = new PostMessageData;
  post_message_data->instance = instance_;
  post_message_data->buffer = array_buffer;

  pp::Module::Get()->core()->CallOnMainThread(
      0,
      pp::CompletionCallback(&ScreenshotTask::MainThreadRun,
                             post_message_data));
}

ImagePtr ScreenshotTask::BufferToImage() {
  size_t width = buffer_->size().width();
  size_t height = buffer_->size().height();
  ImagePtr image(imImageCreate(
      width, height,
      IM_RGB,
      IM_BYTE));
  if (image == NULL) {
    printf("imImageCreate failed.\n");
    return ImagePtr();
  }

  uint32_t* data = buffer_->data();
  for (size_t src_y = 0; src_y < height; ++src_y)
  for (size_t x = 0; x < width; ++x) {
    size_t dst_y = height - 1 - src_y;
    uint32_t src_pixel = data[src_y * width + x];
    uint8_t r = src_pixel >> 16;
    uint8_t g = src_pixel >> 8;
    uint8_t b = src_pixel;

    static_cast<char*>(image->data[0])[dst_y * width + x] = r;
    static_cast<char*>(image->data[1])[dst_y * width + x] = g;
    static_cast<char*>(image->data[2])[dst_y * width + x] = b;
  }

  buffer_.reset();

  return image;
}

FilePtr ScreenshotTask::WriteImageToFile(imBinMemoryFileName* filename,
                                         ImagePtr image) {
  int err;
  FilePtr file(imFileNew(reinterpret_cast<const char*>(filename),
                         config_.file_format.c_str(), &err));
  if (err != IM_ERR_NONE) {
    printf("imFileNew failed. Error: %d\n", err);
    return FilePtr();
  }

  err = imFileSaveImage(file.get(), image.get());
  if (err != IM_ERR_NONE) {
    printf("imFileSaveImage. Error: %d\n", err);
    return FilePtr();
  }

  return file;
}

pp::VarArrayBuffer ScreenshotTask::FileToArrayBuffer(
    imBinMemoryFileName* filename,
    uint32_t request_id,
    FilePtr file) {
  imBinFile* binfile = static_cast<imBinFile*>(imFileHandle(file.get(), 0));
  size_t size = imBinFileSize(binfile);

  pp::VarArrayBuffer array_buffer(sizeof(request_id) + size);
  char* var_data = static_cast<char*>(array_buffer.Map());
  memcpy(var_data, &request_id, sizeof(request_id));
  memcpy(var_data + sizeof(request_id), filename->buffer, size);
  array_buffer.Unmap();

  return array_buffer;
}

void ScreenshotTask::MainThreadRun(void* user_data, int32_t result) {
  PostMessageData* data = static_cast<PostMessageData*>(user_data);
  data->instance->PostMessage(data->buffer);
  delete data;
}
