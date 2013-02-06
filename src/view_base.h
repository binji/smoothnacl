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

#ifndef VIEW_BASE_H_
#define VIEW_BASE_H_

#include <ppapi/cpp/size.h>

namespace pp {
class Instance;
class Point;
class View;
}  // namespace pp

class ViewBase {
 public:
  virtual ~ViewBase() {}
  virtual bool DidChangeView(pp::Instance* instance, const pp::View& view) = 0;
  virtual pp::Size GetSize() const = 0;

  // Helper functions.
  pp::Point ScreenToSim(const pp::Point& p, const pp::Size& sim_size) const;
  void GetScreenToSimScale(const pp::Size& sim_size, double* out_scale,
                           int* out_xoffset, int* out_yoffset) const;
};

#endif  // VIEW_BASE_H_
