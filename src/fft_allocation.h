// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FFT_ALLOCATION_H_
#define FFT_ALLOCATION_H_

#include <fftw3.h>
#include "ppapi/cpp/size.h"

struct ReduceSizeForComplex {};

template<typename T>
class FftAllocation {
 public:
  explicit FftAllocation(const pp::Size& size)
      : size_(size) {
    count_ = size.width() * size.height();
    data_ = static_cast<T*>(fftw_malloc(sizeof(T) * count_));
  }

  FftAllocation(const pp::Size& size, ReduceSizeForComplex)
      : size_(size) {
    count_ = size.width() * (size.height() / 2 + 1);
    data_ = static_cast<T*>(fftw_malloc(sizeof(T) * count_));
  }

  ~FftAllocation() {
    fftw_free(data_);
  }

  T& operator [](size_t index) { return data_[index]; }
  const T& operator [](size_t index) const { return data_[index]; }

  T* data() { return data_; }
  const T* data() const { return data_; }
  pp::Size size() const { return size_; }
  int count() const { return count_; }
  T* begin() { return data_; }
  const T* begin() const { return data_; }
  T* end() { return data_ + count_; }
  const T* end() const { return data_ + count_; }

 private:
  FftAllocation(const FftAllocation&);  // undefined
  FftAllocation& operator =(const FftAllocation&);  // undefined

  T* data_;
  pp::Size size_;
  size_t count_;
};

typedef FftAllocation<double> AlignedReals;
typedef FftAllocation<fftw_complex> AlignedComplexes;

#endif  // FFT_ALLOCATION_H_
