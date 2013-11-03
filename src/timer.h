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

#include <sys/time.h>
#include <stdio.h>

struct Timer {
  Timer(const char* name) :
      name(name) {
    gettimeofday(&start_time, NULL);
  }

  ~Timer() {
    struct timeval end_time;
    gettimeofday(&end_time, NULL);

    unsigned long long us = (end_time.tv_sec - start_time.tv_sec) * 1000000;
    us += (end_time.tv_usec - start_time.tv_usec);
    //printf("%s: %lldus\n", name, us);
  }

  struct timeval start_time;
  const char* name;
};

#define TIME(x) do { Timer t(#x); x; } while(0)
