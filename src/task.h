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
      : function_(function) {}

  virtual void Run(T* thread) {
    function_(thread);
  }

 private:
  std::function<FunctionType> function_;
};

template <typename T>
class MemberFunctionTask : public Task<T> {
 public:
  typedef void FunctionType(T*);
  explicit MemberFunctionTask(const std::function<FunctionType>& function)
      : function_(function) {}

  virtual void Run(T* thread) {
    function_(thread);
  }

 private:
  std::function<FunctionType> function_;
};

template<typename T>
Task<T>* MakeFunctionTask(void (*func)(T*)) {
  return new FunctionTask<T>(std::bind(func, std::placeholders::_1));
}

template<typename T, typename A1,
                     typename P1>
Task<T>* MakeFunctionTask(void (*func)(T*, A1), const P1& p1) {
  return new FunctionTask<T>(std::bind(func, std::placeholders::_1, p1));
}

template<typename T, typename A1, typename A2,
                     typename P1, typename P2>
Task<T>* MakeFunctionTask(void (*func)(T*, A1, A2), const P1& p1,
                          const P2& p2) {
  return new FunctionTask<T>(std::bind(func, std::placeholders::_1, p1,
                                       p2));
}

template<typename T, typename A1, typename A2, typename A3,
                     typename P1, typename P2, typename P3>
Task<T>* MakeFunctionTask(void (*func)(T*, A1, A2, A3), const P1& p1,
                          const P2& p2, const P3& p3) {
  return new FunctionTask<T>(std::bind(func, std::placeholders::_1, p1,
                                       p2, p3));
}

template<typename T, typename A1, typename A2, typename A3, typename A4,
                     typename P1, typename P2, typename P3, typename P4>
Task<T>* MakeFunctionTask(void (*func)(T*, A1, A2, A3, A4), const P1& p1,
                          const P2& p2, const P3& p3, const P4& p4) {
  return new FunctionTask<T>(std::bind(func, std::placeholders::_1, p1,
                                       p2, p3, p4));
}

template<typename T>
Task<T>* MakeFunctionTask(void (T::*func)()) {
  return new MemberFunctionTask<T>(std::bind(func, std::placeholders::_1));
}

template<typename T, typename A1,
                     typename P1>
Task<T>* MakeFunctionTask(void (T::*func)(A1), const P1& p1) {
  return new MemberFunctionTask<T>(std::bind(func, std::placeholders::_1, p1));
}

template<typename T, typename A1, typename A2,
                     typename P1, typename P2>
Task<T>* MakeFunctionTask(void (T::*func)(A1, A2), const P1& p1, const P2& p2) {
  return new MemberFunctionTask<T>(std::bind(func, std::placeholders::_1, p1,
                                             p2));
}

template<typename T, typename A1, typename A2, typename A3,
                     typename P1, typename P2, typename P3>
Task<T>* MakeFunctionTask(void (T::*func)(A1, A2, A3), const P1& p1,
                          const P2& p2, const P3& p3) {
  return new MemberFunctionTask<T>(std::bind(func, std::placeholders::_1, p1,
                                             p2, p3));
}

template<typename T, typename A1, typename A2, typename A3, typename A4,
                     typename P1, typename P2, typename P3, typename P4>
Task<T>* MakeFunctionTask(void (T::*func)(A1, A2, A3, A4), const P1& p1,
                       const P2& p2, const P3& p3, const P4& p4) {
  return new MemberFunctionTask<T>(std::bind(func, std::placeholders::_1, p1,
                                             p2, p3, p4));
}

#endif  // TASK_H_
