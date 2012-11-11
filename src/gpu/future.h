// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_FUTURE_H_
#define GPU_FUTURE_H_

#include <assert.h>
#include <GLES2/gl2.h>
#include <memory>
#include <utility>

namespace gpu {

template<typename T>
class Future {
 public:
  Future();

  // Allow implicit conversion.
  Future(const T& value);

  const T& value() const;
  void set_value(const T& value);
  bool has_value() const;

 private:
  // Pair of T and whether T has been set.
  typedef std::pair<T, bool> Data;
  std::shared_ptr<Data> p_;
};


template<typename T>
Future<T>::Future()
    : p_(new Data(T(), false)) {
}

template<typename T>
Future<T>::Future(const T& value)
    : p_(new Data(T(value), true)) {
}

template<typename T>
const T& Future<T>::value() const {
  assert(has_value());
  return p_->first;
}

template<typename T>
void Future<T>::set_value(const T& value) {
  assert(!has_value());
  p_->first = value;
  p_->second = true;
}

template<typename T>
bool Future<T>::has_value() const {
  return p_->second;
}

typedef Future<GLuint> ID;
typedef Future<GLint> Location;

}  // namespace gpu

#endif  // GPU_FUTURE_H_
