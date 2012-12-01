// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "screenshot_task.h"
#include <im_format_all.h>
#include <im_image.h>
#include <im_process_loc.h>
#include <im_process_pnt.h>
#include <memory>
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

struct ImageDeleter {
  void operator ()(imImage* image) {
    imImageDestroy(image);
  }
};
typedef std::unique_ptr<imImage, ImageDeleter> ImagePtr;

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
  size_t width = buffer_->size().width();
  size_t height = buffer_->size().height();
  ImagePtr image(imImageCreate(
      width, height,
      IM_RGB,
      IM_BYTE));
  if (image == NULL) {
    printf("imImageCreate failed.\n");
    return;
  }

  uint32_t* data = buffer_->data();
  for (size_t src_y = 0; src_y < height; ++src_y)
  for (size_t x = 0; x < width; ++x) {
    size_t dst_y = height - src_y;
    uint32_t src_pixel = data[src_y * width + x];
    uint8_t r = src_pixel >> 16;
    uint8_t g = src_pixel >> 8;
    uint8_t b = src_pixel;

    static_cast<char*>(image->data[0])[dst_y * width + x] = r;
    static_cast<char*>(image->data[1])[dst_y * width + x] = g;
    static_cast<char*>(image->data[2])[dst_y * width + x] = b;
  }

  ImagePtr reduced_image(imImageCreate(width / 2, height / 2, IM_RGB, IM_BYTE));
  if (reduced_image == NULL) {
    printf("imImageCreate failed.\n");
    return;
  }

  int err = imProcessReduce(image.get(), reduced_image.get(), 1);
  if (err == 0) {
    printf("imProcessReduce failed.\n");
    return;
  }

  ImagePtr cropped_image(imImageCreate(width / 4, height / 4, IM_RGB, IM_BYTE));
  if (cropped_image == NULL) {
    printf("imImageCreate failed.\n");
    return;
  }

  imProcessCrop(reduced_image.get(), cropped_image.get(),
                width / 8, height / 8);

  float params[2];
  params[0] = 10;  // Brightness shift.
  params[1] = 30;  // Constrast factor.
  imProcessToneGamut(cropped_image.get(), cropped_image.get(),
                     IM_GAMUT_BRIGHTCONT, &params[0]);

  // Write image to Jpeg memory file.
  err;
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

  err = imFileSaveImage(file_, cropped_image.get());
  if (err != IM_ERR_NONE) {
    printf("imFileSaveImage. Error: %d\n", err);
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
