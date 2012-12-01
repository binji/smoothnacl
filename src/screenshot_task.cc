// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "screenshot_task.h"
#include <algorithm>
#include <im_format_all.h>
#include <im_process_loc.h>
#include <im_process_pnt.h>
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

void ImageDeleter::operator ()(imImage* image) const {
  if (image)
    imImageDestroy(image);
}

void FileDeleter::operator ()(imFile* file) const {
  if (file)
    imFileClose(file);
}

void MemoryFilenameDeleter::operator ()(imBinMemoryFileName* filename) const {
  if (filename)
    free(filename->buffer);
  delete filename;
}

// TODO(binji): Move these somewhere better.
imBinFileBase* iBinSystemFileNewFunc() {
  return NULL;
}

imBinFileBase* iBinSystemFileHandleNewFunc() {
  return NULL;
}

void imFormatRegisterInternal() {
  imBinFileSetCurrentModule(IM_MEMFILE);
  imFormatRegisterJPEG();
  imFormatRegisterPNG();
}

ReduceImageOperation::ReduceImageOperation(size_t max_length)
    : max_length_(max_length) {
}

ImagePtr ReduceImageOperation::Run(ImagePtr src) {
  int new_width;
  int new_height;
  if (src->width > src->height) {
    new_width = max_length_;
    new_height = src->height * max_length_ / src->width;
  } else {
    new_width = src->width * max_length_ / src->height;
    new_height = max_length_;
  }

  ImagePtr dst(imImageCreate(new_width, new_height, IM_RGB, IM_BYTE));
  if (dst == NULL) {
    printf("ReduceImageOperation: imImageCreate failed.\n");
    return ImagePtr();
  }

  int err = imProcessReduce(src.get(), dst.get(), 1);
  if (err == 0) {
    printf("ReduceImageOperation: imProcessReduce failed.\n");
    return ImagePtr();
  }

  return dst;
}

CropImageOperation::CropImageOperation(double x_scale, double y_scale,
                                       size_t max_length)
    : x_scale_(x_scale),
      y_scale_(y_scale),
      max_length_(max_length) {
}

ImagePtr CropImageOperation::Run(ImagePtr src) {
  int new_width;
  int new_height;
  if (src->width > src->height) {
    new_width = max_length_;
    new_height = src->height * max_length_ / src->width;
  } else {
    new_width = src->width * max_length_ / src->height;
    new_height = max_length_;
  }

  ImagePtr dst(imImageCreate(new_width, new_height, IM_RGB, IM_BYTE));
  if (dst == NULL) {
    printf("ReduceImageOperation: imImageCreate failed.\n");
    return ImagePtr();
  }

  int x = static_cast<int>((src->width - new_width) * x_scale_);
  int y = static_cast<int>((src->height - new_height) * y_scale_);
  imProcessCrop(src.get(), dst.get(), x, y);

  return dst;
}

BrightnessContrastImageOperation::BrightnessContrastImageOperation(
    double brightness_shift, double contrast_factor)
    : brightness_shift_(brightness_shift),
      contrast_factor_(contrast_factor) {
}

ImagePtr BrightnessContrastImageOperation::Run(ImagePtr src) {
  ImagePtr dst(imImageCreate(src->width, src->height, IM_RGB, IM_BYTE));
  if (dst == NULL) {
    printf("ReduceImageOperation: imImageCreate failed.\n");
    return ImagePtr();
  }

  float params[2];
  params[0] = brightness_shift_;
  params[1] = contrast_factor_;
  imProcessToneGamut(src.get(), dst.get(), IM_GAMUT_BRIGHTCONT, &params[0]);

  return dst;
}

ScreenshotTask::ScreenshotTask(pp::Instance* instance, AlignedUint32* buffer)
    : instance_(instance),
      buffer_(buffer) {
}

void ScreenshotTask::Run(WorkerThread*) {
  ImagePtr image = BufferToImage();
  image = ReduceImageOperation(256).Run(std::move(image));
  image = CropImageOperation(0.5, 0.5, 128).Run(std::move(image));
  image = BrightnessContrastImageOperation(10, 40).Run(std::move(image));

  MemoryFilenamePtr filename(new imBinMemoryFileName);
  filename->buffer = NULL;
  filename->size = 10 * 1024;
  filename->reallocate = 1.5f;

  FilePtr file = WriteImageToFile(filename.get(), std::move(image));
  pp::VarArrayBuffer array_buffer = FileToArrayBuffer(filename.get(),
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
    size_t dst_y = height - src_y;
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
                         "JPEG", &err));
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
    FilePtr file) {
  imBinFile* binfile = static_cast<imBinFile*>(imFileHandle(file.get(), 0));
  size_t size = imBinFileSize(binfile);

  pp::VarArrayBuffer array_buffer(size);
  void* var_data = array_buffer.Map();
  memcpy(var_data, filename->buffer, size);
  array_buffer.Unmap();

  return array_buffer;
}

void ScreenshotTask::MainThreadRun(void* user_data, int32_t result) {
  PostMessageData* data = static_cast<PostMessageData*>(user_data);
  data->instance->PostMessage(data->buffer);
  delete data;
}
