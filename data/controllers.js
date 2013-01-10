/**
 * Copyright (c) 2012 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 **/
"use strict";

var kernelOrder = ['discRadius', 'ringRadius', 'antiAliasRadius'];
var smootherOrder = ['timestep', 'dt', 'b1', 'd1', 'b2', 'd2',
                     'sigmoidMode', 'sigmoid', 'mix', 'sn', 'sm'];

var controller = function ($scope, $timeout) {
  $scope.drawOptions = {
    view: 0,
  };

  // Kernel
  $scope.kernel = {
    discRadius: 0,
    ringRadius: 0,
    antiAliasRadius: 0
  };
  $scope.getKernelMessage = function () {
    var values = [];
    angular.forEach(kernelOrder, function (value) {
      values.push($scope.kernel[value]);
    });

    return 'SetKernel:' + values.join(',');
  };

  // Smoother
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
  $scope.getSmootherMessage = function () {
    var values = [];
    angular.forEach(smootherOrder, function (value) {
      values.push($scope.smoother[value]);
    });

    return 'SetSmoother:' + values.join(',');
  };


  // Palette
  $scope.palette = {
    colorstops: [],
    gradientType: 0
  };
  $scope.getPaletteMessage = function () {
    var values = [$scope.palette.gradientType];
    angular.forEach($scope.palette.colorstops, function (value) {
      values.push(value.color);
      values.push(value.stop);
    });

    return 'SetPalette:' + values.join(',');
  };
  $scope.addColorstop = function () {
    $scope.palette.colorstops.push({
      color: '#ffffff',
      stop: 100
    });
  };
  $scope.removeColorstop = function (index) {
    $scope.palette.colorstops.splice(index, 1);
  };


  $scope.getValues = function () {
    return {
      kernel: $scope.kernel,
      smoother: $scope.smoother,
      palette: $scope.palette
    };
  };

  $scope.$on('presetChanged', function (event, newPreset) {
    $scope.kernel = angular.copy(newPreset.kernel);
    $scope.smoother = angular.copy(newPreset.smoother);
    $scope.palette= angular.copy(newPreset.palette);

    $timeout(function () {
      $scope.clear();
      $scope.splat();
    });
  });

  $scope.clear = function () {
    $scope.$broadcast('clear');
  };

  $scope.splat = function () {
    $scope.$broadcast('splat');
  };
};

var presetController = function ($scope, localStaticPreset) {
  $scope.savePresets = function () {
    localStaticPreset.set($scope.presets);
  };

  $scope.choosePreset = function (presetIndex) {
    if (!presetsLoaded) return;
    $scope.$emit('presetChanged', $scope.presets[presetIndex]);
  };

  $scope.addPreset = function () {
    if (!presetsLoaded) return;
    var presetObject = angular.copy($scope.getValues());
    presetObject.name = $scope.addPresetName;
    presetObject.canRemove = true;

    // Clear the input box.
    $scope.addPresetName = '';

    $scope.$emit('takeScreenshot', [
        'reduce 256',
        'crop 0.5 0.5 128',
        'brightness_contrast 10 40',
    ], function (url) {
      presetObject.imgSrc = url;
      $scope.savePresets();
    });

    $scope.presets.unshift(presetObject);
    $scope.savePresets();
  };

  $scope.removePreset = function (index) {
    if (!presetsLoaded) return;
    $scope.presets.splice(index, 1);
    $scope.savePresets();
  };

  var presetsLoaded = false;
  localStaticPreset.get(function (presets) {
    $scope.presets = presets;
    presetsLoaded = true;
    $scope.choosePreset(0);
  });
};
