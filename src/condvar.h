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

#ifndef COND_VAR_H_
#define COND_VAR_H_

#include <pthread.h>
#include "locked_object.h"

class CondVar {
 public:
  CondVar();
  ~CondVar();

  void Lock();
  void Unlock();
  void Broadcast();
  void Signal();
  void Wait();

 private:
  LockedObject<pthread_cond_t> locked_cond_;
  pthread_cond_t* cond_;
};

#endif  // COND_VAR_H_
