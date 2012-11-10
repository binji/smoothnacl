// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_FUTURE_H_
#define GPU_FUTURE_H_

#include <assert.h>
#include <GLES2/gl2.h>
#include <memory>
#include <stdio.h>
#include <utility>

namespace gpu {

template<typename T>
class PassFuture;

template<typename T>
class Future {
 public:
  Future();
  Future(const Future&);
  Future& operator =(const Future&);
  ~Future();

  Future(const PassFuture<T>&);

  // Allow implicit conversion.
  Future(const T& value);

  const T& value() const;
  void set_value(const T& value);
  bool has_value() const;

 private:
  friend class PassFuture<T>;

  // Pair of T and whether T has been set.
  typedef std::pair<T, bool> Data;
  Data* p_;
  mutable bool owned_;
};

template<typename T>
class PassFuture {
 public:
  // Take ownership.
  PassFuture(const PassFuture& pf)
      : future_(pf.future_) {
    printf("PassFuture(PassFuture& %p): %p", pf.future_.p_, future_.p_);
    future_.owned_ = true;
    pf.future_.owned_ = false;
  }

  // Allow implicit conversion, and take ownership.
  PassFuture(const Future<T>& other)
      : future_(other) {
    printf("PassFuture(Future& %p): %p", other.p_, future_.p_);
    future_.owned_ = true;
    other.owned_ = false;
  }

 private:
  friend class Future<T>;

  Future<T> future_;

  PassFuture();  // Undefined.
  PassFuture& operator =(const PassFuture& pf);  // Undefined.
};


template<typename T>
Future<T>::Future()
    : p_(new Data(T(), false)),
      owned_(true) {
  printf("Future():%p\n", p_);
}

template<typename T>
Future<T>::Future(const Future& other)
    : p_(other.p_),
      owned_(false) {
  printf("Future(const Future& %p):%p\n", other.p_, p_);
}

template<typename T>
Future<T>& Future<T>::operator =(const Future& other) {
  printf("Future& operator =(const Future& %p):%p\n", other.p_, p_);
  p_ = other.p_;
  owned_ = false;
  return *this;
}

template<typename T>
Future<T>::~Future() {
  printf("~Future()", p_);
  if (owned_)
    delete p_;
}

template<typename T>
Future<T>::Future(const PassFuture<T>& pf)
    : p_(pf.future_.p_),
      owned_(true) {
  printf("Future(PassFuture& %p): %p", pf.future_.p_, p_);
  pf.future_.owned_ = false;
}

template<typename T>
Future<T>::Future(const T& value)
    : p_(new Data(T(value), true)),
      owned_(true) {
  printf("Future(T& %d): %p", value, p_);
}

template<typename T>
const T& Future<T>::value() const {
  assert(has_value());
  return p_->first;
}

template<typename T>
void Future<T>::set_value(const T& value) {
  if (has_value()) {
    assert(0);
  }
  //assert(!has_value());
  p_->first = value;
  p_->second = true;
}

template<typename T>
bool Future<T>::has_value() const {
  return p_->second;
}

typedef Future<GLuint> ID;
typedef Future<GLint> Location;
typedef PassFuture<GLuint> PassID;
typedef PassFuture<GLint> PassLocation;

}  // namespace gpu

#endif  // GPU_FUTURE_H_
