// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_FUTURE_H_
#define GPU_FUTURE_H_

#include <memory>

namespace gpu {

template<typename T>
class Future {
 public:
  Future() {}

  // Allow implicit conversion.
  Future(const T& value) : p_(new T(value)) {}

  const T& value() const { assert(p_); return *p_; }
  void set_value(const T& value) { assert(!has_value()); *p_ = value; }
  bool has_value() const { return p_; }

 private:
  std::shared_ptr<T> p_;
};

}  // namespace gpu

#endif  // GPU_FUTURE_H_
