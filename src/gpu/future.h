// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_FUTURE_H_
#define GPU_FUTURE_H_

#include <assert.h>
#include <utility>

namespace gpu {

struct TakeOwnership {};

template <typename T>
class PassFuture;

template<typename T>
class Future {
 public:
  Future();
  ~Future();
  Future(const Future& other);
  Future(const Future& other, TakeOwnership);
  Future(const PassFuture<T>& other);
  Future& operator =(const Future& other);
  Future& operator =(const PassFuture<T>& other);
  // Allow implicit conversion.
  Future(const T& value);
  const T& value() const;
  void set_value(const T& value);
  bool has_value() const;

 private:
  bool is_pointer() const;

  // Pair of T and whether T has been set.
  typedef std::pair<T, bool> Data;
  enum State {
    IS_VALUE,
    IS_UNOWNED_POINTER,
    IS_OWNED_POINTER,
  };
  union {
    Data* p_;
    T value_;
  };
  mutable State state_;

  friend class PassFuture<T>;
};

template <typename T>
class PassFuture {
 public:
  // Allow implicit conversion.
  PassFuture(const Future<T>& other);
  PassFuture(const PassFuture& other);
  PassFuture& operator =(const PassFuture& other);

 private:
  Future<T> future_;

  friend class Future<T>;
};


template <typename T>
Future<T>::Future()
    : p_(new Data(T(), false)),
      state_(IS_OWNED_POINTER) {
}

template <typename T>
Future<T>::~Future() {
  if (state_ == IS_OWNED_POINTER) {
    delete p_;
  }
}

template <typename T>
Future<T>::Future(const Future& other) {
  switch (other.state_) {
    case IS_VALUE:
      value_ = other.value_;
      state_ = other.state_;
      break;

    case IS_UNOWNED_POINTER:
      p_ = other.p_;
      state_ = other.state_;
      break;

    case IS_OWNED_POINTER:
      p_ = other.p_;
      state_ = IS_UNOWNED_POINTER;
      break;

    default:
      assert(0);
  }
}

template <typename T>
Future<T>::Future(const Future& other, TakeOwnership) {
  switch (other.state_) {
    case IS_VALUE:
      value_ = other.value_;
      state_ = other.state_;
      break;

    case IS_UNOWNED_POINTER:
      p_ = other.p_;
      state_ = other.state_;
      break;

    case IS_OWNED_POINTER:
      p_ = other.p_;
      state_ = IS_OWNED_POINTER;
      other.state_ = IS_UNOWNED_POINTER;
      break;

    default:
      assert(0);
  }
}

template <typename T>
Future<T>::Future(const PassFuture<T>& other) {
  switch (other.future_.state_) {
    case IS_VALUE:
      value_ = other.future_.value_;
      state_ = other.future_.state_;
      break;

    case IS_UNOWNED_POINTER:
      p_ = other.future_.p_;
      state_ = other.future_.state_;
      break;

    case IS_OWNED_POINTER:
      p_ = other.future_.p_;
      state_ = IS_OWNED_POINTER;
      other.future_.state_ = IS_UNOWNED_POINTER;
      break;

    default:
      assert(0);
  }
}

template <typename T>
Future<T>& Future<T>::operator =(const Future& other) {
  if (state_ == IS_OWNED_POINTER)
    delete p_;

  switch (other.state_) {
    case IS_VALUE:
      value_ = other.value_;
      state_ = other.state_;
      break;

    case IS_UNOWNED_POINTER:
      p_ = other.p_;
      state_ = other.state_;
      break;

    case IS_OWNED_POINTER:
      p_ = other.p_;
      state_ = IS_UNOWNED_POINTER;
      break;

    default:
      assert(0);
  }

  return *this;
}

template <typename T>
Future<T>& Future<T>::operator =(const PassFuture<T>& other) {
  if (state_ == IS_OWNED_POINTER)
    delete p_;

  switch (other.future_.state_) {
    case IS_VALUE:
      value_ = other.future_.value_;
      state_ = other.future_.state_;
      break;

    case IS_UNOWNED_POINTER:
      p_ = other.future_.p_;
      state_ = other.future_.state_;
      break;

    case IS_OWNED_POINTER:
      p_ = other.future_.p_;
      state_ = IS_OWNED_POINTER;
      other.future_.state_ = IS_UNOWNED_POINTER;
      break;

    default:
      assert(0);
  }

  return *this;
}

template <typename T>
Future<T>::Future(const T& value)
    : value_(value),
      state_(IS_VALUE) {
}

template <typename T>
const T& Future<T>::value() const {
  if (is_pointer()) {
    assert(has_value());
    return p_->first;
  } else {
    return value_;
  }
}

template <typename T>
void Future<T>::set_value(const T& value) {
  assert(is_pointer() && !has_value());
  p_->first = value;
  p_->second = true;
}

template <typename T>
bool Future<T>::has_value() const {
  return !is_pointer() || p_->second;
}

template <typename T>
bool Future<T>::is_pointer() const {
  return state_ != IS_VALUE;
}

template <typename T>
PassFuture<T>::PassFuture(const Future<T>& other)
    : future_(other, TakeOwnership()) {
}

template <typename T>
PassFuture<T>::PassFuture(const PassFuture& other)
    : future_(other) {  // Take ownership.
}

template <typename T>
PassFuture<T>& PassFuture<T>::operator =(const PassFuture& other) {
  future_ = other;  // Take ownership.
}

// Copied from GLES2/gl2.h
typedef int              GLint;
typedef unsigned int     GLuint;

typedef Future<GLuint> ID;
typedef Future<GLint> Location;
typedef PassFuture<GLuint> PassID;
typedef PassFuture<GLint> PassLocation;

}  // namespace gpu

#endif  // GPU_FUTURE_H_
