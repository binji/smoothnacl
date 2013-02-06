/**
 * Copyright (c) 2012 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 **/
"use strict";

(function () {

  var colorsService = function () {

    var nonNegativeMod = function(x, y) {
      return ((x % y) + y) % y;
    };

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

    var mixColor = function (c0, c1, x) {
      var a0 = parseColor(c0);
      var a1 = parseColor(c1);
      return makeColor(mix(a0[0], a1[0], x),
                       mix(a0[1], a1[1], x),
                       mix(a0[2], a1[2], x));
    };

    var getPaletteColor = function (palette, value) {
      var colorstops = palette.colorstops;

      if (!colorstops || colorstops.length === 0)
        return '#000000';

      var stops = [];
      angular.forEach(colorstops, function (colorstop) {
        stops.push(colorstop.stop);
      });

      var minStop = stops[0];
      var maxStop = Math.max.apply(null, stops);
      var stopRange = maxStop - minStop;

      if (palette.gradientType == 1)
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
        return mixColor(colorstops[i].color, colorstops[i+1].color,
                        mixFraction);
      }

      return colorstops[colorstops.length - 1].color;
    };

    return {
      parseColor: parseColor,
      makeColor: makeColor,
      mixColor: mixColor,
      getPaletteColor: getPaletteColor,
    };
  };

  window.module
      .factory('colors', colorsService);
})();
