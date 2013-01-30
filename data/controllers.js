/**
 * Copyright (c) 2012 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 **/
"use strict";

var kernelOrder = ['discRadius', 'ringRadius', 'antiAliasRadius'];
var smootherOrder = ['timestep', 'dt', 'b1', 'd1', 'b2', 'd2',
                     'sigmoidMode', 'sigmoid', 'mix', 'sn', 'sm'];

var nonNegativeMod = function(x, y) {
  return ((x % y) + y) % y;
}

var mixColor = function (c0, c1, x) {
  var parseColor = function (c) {
    // Assume the color is in "#xxxxxx" format.
    var i = parseInt(c.substr(1), 16);
    var r = (i >> 16) & 0xff;
    var g = (i >> 8) & 0xff;
    var b = (i >> 0) & 0xff;
    return [r, g, b];
  };

  var makeColor = function (r, g, b) {
    var hex = ((r << 16) | (g << 8) | b).toString(16)
    // Add leading zeroes.
    return '#' + ('000000' + hex).substr(-6);
  };

  var mix = function (t0, t1, x) {
    return t0 * (1 - x) + t1 * x;
  };

  var a0 = parseColor(c0);
  var a1 = parseColor(c1);
  return makeColor(mix(a0[0], a1[0], x),
                   mix(a0[1], a1[1], x),
                   mix(a0[2], a1[2], x));
}

var controller = function ($scope) {
  $scope.drawOptions = {
    view: 0,
  };

  $scope.brush = {
    radius: 10,
    color: 1.0,
  };
  $scope.getBrushMessage = function () {
    return 'SetBrush:' + $scope.brush.radius + ',' + $scope.brush.color;
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
  $scope.getColor = function (value) {
    var colorstops = $scope.palette.colorstops;

    if (!colorstops || colorstops.length === 0)
      return '#000000';

    var stops = [];
    angular.forEach(colorstops, function (colorstop) {
      stops.push(colorstop.stop);
    });

    var minStop = stops[0];
    var maxStop = Math.max.apply(null, stops);
    var stopRange = maxStop - minStop;

    if ($scope.palette.gradientType == 1)
      value = nonNegativeMod(value - minStop, stopRange) + minStop;

    if (value < minStop)
      return colorstops[0].color;

    var rangeMin = 0;
    for (var i = 0; i < colorstops.length - 1; ++i) {
      rangeMin = Math.max(rangeMin, stops[i]);
      var rangeMax = stops[i + 1];
      if (rangeMin >= rangeMax)
        continue;

      if (value < rangeMin || value > rangeMax)
        continue;

      var mixFraction = (value - rangeMin) / (rangeMax - rangeMin);
      return mixColor(colorstops[i].color, colorstops[i+1].color, mixFraction);
    }

    return colorstops[colorstops.length - 1].color;
  };


  $scope.getValues = function (fromPreset) {
    if (!fromPreset) {
      fromPreset = $scope;
    }

    return {
      kernel: fromPreset.kernel,
      smoother: fromPreset.smoother,
      palette: fromPreset.palette
    };
  };

  $scope.$on('presetChanged', function (event, newPreset) {
    $scope.kernel = angular.copy(newPreset.kernel);
    $scope.smoother = angular.copy(newPreset.smoother);
    $scope.palette= angular.copy(newPreset.palette);

    // When the preset changes, we want to clear/splat -- but only after the
    // values have changed to the new values. Broadcast those values with the
    // message, but if it is unset, perform the clear/splat immediately.
    var values = angular.copy($scope.getValues(newPreset));
    $scope.$broadcast('clearSplat', values);
  });

  $scope.clear = function () {
    $scope.$broadcast('clear');
  };

  $scope.splat = function () {
    $scope.$broadcast('splat');
  };
};

var presetController = function ($scope, localStaticPreset) {
  var presetsLoaded = false;

  $scope.savePresets = function () {
    if (!presetsLoaded) return;
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

  localStaticPreset.get(function (presets) {
    $scope.presets = presets;
    presetsLoaded = true;
    $scope.choosePreset(0);
  });
};
