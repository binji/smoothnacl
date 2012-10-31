// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
