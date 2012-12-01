// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IMAGE_OPERATIONS_H_
#define IMAGE_OPERATIONS_H_

#include "im_utils.h"

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

#endif  // IMAGE_OPERATIONS_H_
