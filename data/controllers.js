/**
 * Copyright (c) 2012 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 **/
"use strict";

var kernelOrder = ['discRadius', 'ringRadius', 'antiAliasRadius'];
var smootherOrder = ['timestep', 'dt', 'b1', 'd1', 'b2', 'd2',
                     'sigmoidMode', 'sigmoid', 'mix', 'sn', 'sm'];

var controller = function ($scope) {
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
    $scope.kernel = newPreset.kernel;
    $scope.smoother = newPreset.smoother;
    $scope.palette= newPreset.palette;

    $scope.$broadcast('clearSplat');
  });
};

var presetController = function ($scope) {
  var presets = [
    ["Subdividers",[15.2,32.1,7.6],[3,0.329,0.15,0.321,0.145,0.709,3,2,4,0.269,0.662],[0,"#000000",3,"#f5f5c1",12,"#158a34",68,"#89e681",100]],
    ["Green Hex",[15.2,32.1,5],[3,0.273,0.117,0.288,0.243,0.348,3,2,4,0.269,0.662],[1,"#000000",3,"#f5f5c1",8,"#158a34",17,"#89e681",20]],
    ["Fire Jellies",[4,12,1],[2,0.115,0.269,0.523,0.34,0.746,3,4,4,0.028,0.147],[0,"#36065e",0,"#c24242",77,"#8a19b0",91,"#ff9900",99,"#f5c816",99]],
    ["Magical Maze",[4,12,1],[3,0.12,0.218,0.267,0.365,0.445,3,4,4,0.028,0.147],[0,"#000000",0,"#0f8a84",38,"#f5f5c1",43,"#158a34",70,"#89e681",100]],
    ["Waterbug Gliders",[4,12,1],[0,0.09,0.276,0.27,0.365,0.445,1,4,4,0.028,0.147],[0,"#93afd9",11,"#9cf0ff",92,"#edfdff",100]],
    ["Stitches 'n' Jitters",[10.4,12,1],[2,0.082,0.302,0.481,0.35,0.749,2,3,4,0.028,0.147],[0,"#000000",11,"#ffffff",22,"#19a68a",85,"#6b0808",98]],
    ["Blink Tubes",[7.8,27.2,2.6],[3,0.21,0.714,0.056,0.175,0.838,2,0,2,0.132,0.311],[0,"#0a1340",0,"#ffffff",55,"#4da8a3",83,"#2652ab",99,"#2f1e75",46]],
    ["Oil Slick",[4,12,1],[2,0.115,0.269,0.496,0.34,0.767,3,4,4,0.028,0.147],[0,"#b8cfcf",0,"#3f5a5c",77,"#1a330a",91,"#c0e0dc",99]],
    ["Worms",[10.6,31.8,1],[1,0.157,0.092,0.256,0.098,0.607,3,4,4,0.015,0.34],[0,"#4d3e3e",0,"#9a1ac9",77,"#aaf09e",100]],
  ];

  function getPresetObject(preset) {
    var result = {
      name: preset[0],
      canRemove: false,
      kernel: {},
      smoother: {},
      palette: {
        gradientType: preset[3][0],
        colorstops: [],
      }
    };

    angular.forEach(kernelOrder, function (value, index) {
      result.kernel[value] = preset[1][index];
    });
    angular.forEach(smootherOrder, function (value, index) {
      result.smoother[value] = preset[2][index];
    });
    for (var i = 1; i < preset[3].length; i += 2) {
      result.palette.colorstops.push({
        color: preset[3][i],
        stop: preset[3][i + 1],
      });
    }

    return result;
  };

  $scope.presets = [];
  angular.forEach(presets, function (preset, index) {
    var presetObject = getPresetObject(preset);
    presetObject.imgSrc = 'preset' + index + '.jpg';
    $scope.presets.push(presetObject);
  });

  $scope.setPreset = function (presetIndex) {
    $scope.$emit('presetChanged', $scope.presets[presetIndex]);
  };

  $scope.addPreset = function () {
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
    });

    $scope.presets.unshift(presetObject);

    return false;
  };

  $scope.removePreset = function (index) {
    $scope.presets.splice(index, 1);
  };

  $scope.setPreset(0);
};
