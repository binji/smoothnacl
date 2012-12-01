// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "image_operation.h"
#include <im_process_loc.h>
#include <im_process_pnt.h>
#include <stdio.h>

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
