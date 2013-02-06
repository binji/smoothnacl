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
