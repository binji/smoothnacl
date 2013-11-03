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

#ifndef FFT_ALLOCATION_H_
#define FFT_ALLOCATION_H_

#include <algorithm>

#include <stdint.h>
#include <string.h>
#include "ppapi/cpp/size.h"

#include "fftw.h"

struct ReduceSizeForComplex {};

template<typename T>
class FftAllocation {
 public:
  typedef T ElementType;

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

  FftAllocation(const FftAllocation& other)
      : size_(other.size_) {
    count_ = other.count_;
    data_ = static_cast<T*>(fftw_malloc(sizeof(T) * count_));
    memcpy(data_, other.data_, sizeof(T) * count_);
  }

  ~FftAllocation() {
    fftw_free(data_);
  }

  T& operator [](size_t index) { return data_[index]; }
  const T& operator [](size_t index) const { return data_[index]; }

  T* data() { return data_; }
  const T* data() const { return data_; }
  pp::Size size() const { return size_; }
  size_t byte_size() const { return sizeof(T) * count_; }
  int count() const { return count_; }
  T* begin() { return data_; }
  const T* begin() const { return data_; }
  T* end() { return data_ + count_; }
  const T* end() const { return data_ + count_; }

  void swap(FftAllocation& other) {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    std::swap(count_, other.count_);
  }

 private:
  FftAllocation& operator =(const FftAllocation&);  // undefined

  T* data_;
  pp::Size size_;
  size_t count_;
};

typedef FftAllocation<uint32_t> AlignedUint32;
typedef FftAllocation<real> AlignedReals;
typedef FftAllocation<float> AlignedFloats;
typedef FftAllocation<fftw_complex> AlignedComplexes;

#endif  // FFT_ALLOCATION_H_
