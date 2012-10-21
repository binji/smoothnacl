#ifndef TASK_H_
#define TASK_H_

#include <functional>

class SmoothlifeThread;

class Task {
 public:
  virtual ~Task() {}
  virtual void Run(SmoothlifeThread* thread) = 0;
};

class FunctionTask : public Task {
 public:
  typedef void FunctionType(SmoothlifeThread*);
  FunctionTask(const std::function<FunctionType>& function)
      : function_(function) {
  }

  virtual void Run(SmoothlifeThread* thread) {
    function_(thread);
  }

 private:
  std::function<FunctionType> function_;
};

template<typename F>
Task* MakeFunctionTask(F func) {
  return new FunctionTask(std::bind(func, std::placeholders::_1));
}

template<typename F, typename P1>
Task* MakeFunctionTask(F func, const P1& p1) {
  return new FunctionTask(std::bind(func, std::placeholders::_1, p1));
}

template<typename F, typename P1, typename P2>
Task* MakeFunctionTask(F func, const P1& p1, const P2& p2) {
  return new FunctionTask(std::bind(func, std::placeholders::_1, p1, p2));
}

template<typename F, typename P1, typename P2, typename P3>
Task* MakeFunctionTask(F func, const P1& p1, const P2& p2, const P3& p3) {
  return new FunctionTask(std::bind(func, std::placeholders::_1, p1, p2, p3));
}

template<typename F, typename P1, typename P2, typename P3, typename P4>
Task* MakeFunctionTask(F func, const P1& p1, const P2& p2, const P3& p3,
                       const P4& p4) {
  return new FunctionTask(std::bind(func, std::placeholders::_1, p1, p2, p3,
                                    p4));
}

#endif  // TASK_H_
