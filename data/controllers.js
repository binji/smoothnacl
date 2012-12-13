/**
 * Copyright (c) 2012 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 **/
"use strict"

var controller = function ($scope) {
  $scope.drawOptions = {
    view: 0,
  };

  $scope.kernel = {
    discRadius: 3.0,
    ringRadius: 9.0,
    antiAliasRadius: 3.0
  };

  $scope.smoother = {
    timestep: 0,
    dt: 0.0,
    b1: 0.0,
    d1: 0.0,
    b2: 0.0,
    d2: 0.0,
    sigmoidMode: 0,
    sigmoid: 0,
    mix: 0,
    sn: 0.0,
    sm: 0.0,
  };

  $scope.palette = {
    colorstops: [
      { color: '#000000', stop: 0 },
      { color: '#ffffff', stop: 100 },
    ],
    gradientType: 0
  };

  $scope.addColorstop = function () {
    $scope.palette.colorstops.push({
      color: '#ffffff',
      stop: 100
    });
  };
};
