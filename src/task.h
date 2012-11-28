// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TASK_H_
#define TASK_H_

#include <functional>

template <typename T>
class Task {
 public:
  virtual ~Task() {}
  virtual void Run(T* thread) = 0;
};

template <typename T>
class FunctionTask : public Task<T> {
 public:
  typedef void FunctionType(T*);
  explicit FunctionTask(const std::function<FunctionType>& function)
      : function_(function) {
  }

  virtual void Run(T* thread) {
    function_(thread);
  }

 private:
  std::function<FunctionType> function_;
};

template<typename T>
Task<T>* MakeFunctionTask(void (T::*func)()) {
  return new FunctionTask<T>(std::bind(func, std::placeholders::_1));
}

template<typename T, typename A1,
                     typename P1>
Task<T>* MakeFunctionTask(void (T::*func)(A1), const P1& p1) {
  return new FunctionTask<T>(std::bind(func, std::placeholders::_1, p1));
}

template<typename T, typename A1, typename A2,
                     typename P1, typename P2>
Task<T>* MakeFunctionTask(void (T::*func)(A1, A2), const P1& p1, const P2& p2) {
  return new FunctionTask<T>(std::bind(func, std::placeholders::_1, p1, p2));
}

template<typename T, typename A1, typename A2, typename A3,
                     typename P1, typename P2, typename P3>
Task<T>* MakeFunctionTask(void (T::*func)(A1, A2, A3), const P1& p1,
                          const P2& p2, const P3& p3) {
  return new FunctionTask<T>(std::bind(func, std::placeholders::_1, p1, p2,
                                       p3));
}

template<typename T, typename A1, typename A2, typename A3, typename A4,
                     typename P1, typename P2, typename P3, typename P4>
Task<T>* MakeFunctionTask(void (T::*func)(A1, A2, A3, A4), const P1& p1,
                       const P2& p2, const P3& p3, const P4& p4) {
  return new FunctionTask<T>(std::bind(func, std::placeholders::_1, p1, p2, p3,
                                       p4));
}

#endif  // TASK_H_
