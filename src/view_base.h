// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
