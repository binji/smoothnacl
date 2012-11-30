// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "screenshot_task.h"
#include <im_format_all.h>
#include <im_image.h>
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

uint32_t ArgbToAbgr(uint32_t color) {
  return (color & 0xff00ff00) |
      ((color << 16) & 0xff0000) |
      ((color >> 16) & 0xff);
}

void ArgbToAbgr(AlignedUint32* buffer) {
  uint32_t* data = buffer->data();
  for (size_t i = 0; i < buffer->count(); ++i)
    data[i] = ArgbToAbgr(data[i]);
}

}  // namespace

// TODO(binji): Move these somewhere better.
imBinFileBase* iBinSystemFileNewFunc() {
  return NULL;
}

imBinFileBase* iBinSystemFileHandleNewFunc() {
  return NULL;
}

void imFormatRegisterInternal() {
  imFormatRegisterJPEG();
  imFormatRegisterPNG();
}

ScreenshotTask::ScreenshotTask(pp::Instance* instance, AlignedUint32* buffer)
    : instance_(instance),
      buffer_(buffer),
      file_(NULL),
      filename_(NULL) {
}

ScreenshotTask::~ScreenshotTask() {
  if (filename_)
    free(filename_->buffer);
  delete filename_;
  delete buffer_;
  if (file_)
    imFileClose(file_);
}

void ScreenshotTask::Run(WorkerThread*) {
  ArgbToAbgr(buffer_);

  int err;
  if (imBinFileSetCurrentModule(IM_MEMFILE) == -1) {
    printf("imBinMemoryFileName failed.\n");
    return;
  }

  filename_ = new imBinMemoryFileName;
  filename_->buffer = NULL;
  filename_->size = 10 * 1024;
  filename_->reallocate = 1.5f;

  file_ = imFileNew(reinterpret_cast<const char*>(filename_), "JPEG", &err);
  if (err != IM_ERR_NONE) {
    printf("imFileNew failed. Error: %d\n", err);
    return;
  }

  err = imFileWriteImageInfo(
      file_,
      buffer_->size().width(), buffer_->size().height(),
      IM_RGB | IM_ALPHA | IM_PACKED | IM_TOPDOWN,
      IM_BYTE);
  if (err != IM_ERR_NONE) {
    printf("imFileWriteImageInfo failed. Error: %d\n", err);
    return;
  }

  err = imFileWriteImageData(file_, buffer_->data());
  if (err != IM_ERR_NONE) {
    printf("imFileWriteImageData failed. Error: %d\n", err);
    return;
  }

  imBinFile* binfile = static_cast<imBinFile*>(imFileHandle(file_, 0));
  size_t size = imBinFileSize(binfile);

  PostMessageData* post_message_data = new PostMessageData;
  post_message_data->instance = instance_;
  post_message_data->buffer = pp::VarArrayBuffer(size);
  void* var_data = post_message_data->buffer.Map();
  memcpy(var_data, filename_->buffer, size);
  post_message_data->buffer.Unmap();

  pp::Module::Get()->core()->CallOnMainThread(
      0,
      pp::CompletionCallback(&ScreenshotTask::MainThreadRun,
                             post_message_data));
}

void ScreenshotTask::MainThreadRun(void* user_data, int32_t result) {
  PostMessageData* data = static_cast<PostMessageData*>(user_data);
  data->instance->PostMessage(data->buffer);
  delete data;
}
