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

#include "functions.h"
#include <math.h>

namespace {
  const double kPi = 3.14159265358979323846;
}

double func_hard(double x, double a) {
  if (x>=a) return 1.0; else return 0.0;
}

double func_linear(double x, double a, double ea) {
  if (x < a-ea/2.0) return 0.0;
  else if (x > a+ea/2.0) return 1.0;
  else return (x-a)/ea + 0.5;
}

double func_hermite(double x, double a, double ea) {
  if (x < a-ea/2.0) return 0.0;
  else if (x > a+ea/2.0) return 1.0;
  else {
    double m = (x-(a-ea/2.0))/ea;
    return m*m*(3.0-2.0*m);
  }
}

double func_sin(double x, double a, double ea) {
  if (x < a-ea/2.0) return 0.0;
  else if (x > a+ea/2.0) return 1.0;
  else return sin(kPi*(x-a)/ea)*0.5+0.5;
}

double func_smooth(double x, double a, double ea) {
  return 1.0/(1.0+exp(-(x-a)*4.0/ea));
}
