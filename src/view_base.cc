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

#include "view_base.h"
#include <algorithm>
#include <ppapi/cpp/point.h>
#include <ppapi/cpp/size.h>

pp::Point ViewBase::ScreenToSim(const pp::Point& p,
                                const pp::Size& sim_size) const {
  double scale;
  int x_offset;
  int y_offset;
  GetScreenToSimScale(sim_size, &scale, &x_offset, &y_offset);
  return pp::Point(
      static_cast<int>((p.x() - x_offset) * scale),
      static_cast<int>((p.y() - y_offset) * scale));
}

void ViewBase::GetScreenToSimScale(const pp::Size& sim_size, double* out_scale,
                                   int* out_xoffset, int* out_yoffset) const {
  // Keep the aspect ratio.
  int image_width = GetSize().width();
  int image_height = GetSize().height();
  *out_scale = std::max(
      static_cast<double>(sim_size.width()) / image_width,
      static_cast<double>(sim_size.height()) / image_height);
  *out_xoffset =
      static_cast<int>((image_width - sim_size.width() / *out_scale) / 2);
  *out_yoffset =
      static_cast<int>((image_height - sim_size.height() / *out_scale) / 2);
}
