/**
 * Copyright 2013 Ben Smith. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/
"use strict";

var kernelOrder = ['discRadius', 'ringRadius', 'antiAliasRadius'];
var smootherOrder = ['timestep', 'dt', 'b1', 'd1', 'b2', 'd2',
                     'sigmoidMode', 'sigmoid', 'mix', 'sn', 'sm'];

var controller = function ($scope, colors, $http) {
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
    return colors.getPaletteColor($scope.palette, value);
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

  $scope.upload = function () {
    $scope.$broadcast('getBuffer', function (data) {
      var blob = new Blob([data], {type: 'application/octet-stream'});

      var fd = new FormData();
      fd.append('name', 'Test');
      fd.append('values', JSON.stringify($scope.getValues()));
      fd.append('buffer', blob);
      $http({
        method: 'POST',
        url: 'http://localhost:8080/api/patterns',
        data: fd,
        headers: {
          // Auto-generate multipart/form-data with boundary.
          'Content-Type': undefined,
        },
        transformRequest: angular.identity
      }).success(function (response) {
        console.log('Success!');
      }).error(function (response) {
        console.log('error...');
      });
    });
  };
};

var presetController = function ($scope, staticPreset, localStoragePreset) {
  var localStorageLoaded = false;
  var saveLocalStorageWhenLoaded = false;

  $scope.savePresets = function () {
    if (!localStorageLoaded) {
      saveLocalStorageWhenLoaded = true;
      return;
    }
    localStoragePreset.set($scope.presets);
  };

  $scope.choosePreset = function (presetIndex) {
    $scope.$emit('presetChanged', $scope.presets[presetIndex]);
  };

  $scope.addPreset = function () {
    var presetObject = angular.copy($scope.getValues());
    presetObject.name = $scope.addPresetName;
    presetObject.canRemove = true;
    presetObject.shouldSave = true;

    // Clear the input box.
    $scope.addPresetName = '';

    $scope.$emit('takeScreenshot', 'jpeg', [
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
    $scope.presets.splice(index, 1);
    $scope.savePresets();
  };

  staticPreset.get(function (presets) {
    $scope.presets = presets;
    $scope.choosePreset(0);
  });

  localStoragePreset.get(function (presets) {
    $scope.presets = presets.concat($scope.presets);
    localStorageLoaded = true;
    // If we have a queued save call, now save it.
    if (saveLocalStorageWhenLoaded) {
      $scope.savePresets();
      saveLocalStorageWhenLoaded = false;
    }
  });
};
