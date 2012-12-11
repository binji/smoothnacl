/**
 * Copyright (c) 2012 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 **/
"use strict"

angular.module('directives', [])
  .directive('buttonset', function () {
    return {
      require: '?ngModel',
      restrict: 'E',  // Element only.
      replace: true,
      template: '<div/>',
      link: function (scope, iElement, iAttrs, ngModel) {
        var name = iAttrs.name;
        var values = iAttrs.values.split(' ');
        for (var i = 0; i < values.length; ++i) {
          var valueText = values[i];
          var optionId = name.replace(' ', '_') + '_' + i;
          var labelEl = $('<label/>').attr('for', optionId)
                                     .text(valueText);
          var optionEl = $('<input/>').attr('type', 'radio')
                                      .attr('id', optionId)
                                      .attr('name', name)
                                      .data('value', i);
          iElement.append(labelEl).append(optionEl);
        }

        iElement.buttonset();

        if (ngModel) {
          ngModel.$render = function () {
            var value = ngModel.$viewValue;
            iElement.children('input').removeAttr('checked');
            iElement.children('input').eq(value).attr('checked', 'checked');
            iElement.buttonset('refresh');
          };

          iElement.bind('change', function (e) {
            scope.$apply(function () {
              var value = iElement.children('input:checked').data('value');
              ngModel.$setViewValue(value);
            });
          });
        }
      },
    };
  })
  .directive('slider', function () {
    return {
      require: '?ngModel',
      restrict: 'E',  // Element only.
      replace: true,
      template: '<div/>',
      link: function (scope, iElement, iAttrs, ngModel) {
        var prec = +iAttrs.prec;
        var mult = Math.pow(10, prec);
        var value = iAttrs.min * mult;

        iElement.slider({
          value: value,
          min: iAttrs.min * mult,
          max: iAttrs.max * mult,
          range: 'min'
        });

        if (ngModel) {
          ngModel.$render = function () {
            iElement.slider('value', ngModel.$viewValue * mult);
          };

          iElement.bind('slide', function (e, ui) {
            scope.$apply(function () {
              ngModel.$setViewValue(ui.value / mult);
            });
          });
        }
      }
    };
  })
  .directive('sliderRow', function () {
    return {
      restrict: 'E',
      replace: true,
      template:
        '<div class="range setting-row">' +
          '<label></label>' +
          '<div class="value">' +
            '<span></span>' +
            '<slider/>' +
          '</div>' +
        '</div>',
      compile: function (tElement, tAttrs) {
        var model = tAttrs.model;
        var label = tElement.find('label').text(tAttrs.label);
        var span = tElement.find('span');
        span.text('{{' + model + ' | number:' + tAttrs.prec + '}}');
        var slider = tElement.find('slider');
        slider.attr({
          'data-ng-model': model,
          'data-min': tAttrs.min,
          'data-max': tAttrs.max,
          'data-prec': tAttrs.prec,
        });

        return function postLink(scope, iElement, iAttrs) {};
      },
    };
  })
  .directive('buttonsetRow', function () {
    return {
      restrict: 'E',
      replace: true,
      template:
        '<div class="enum setting-row">' +
          '<label></label>' +
          '<buttonset class="value">' +
          '</buttonset>' +
        '</div>',
      compile: function (tElement, tAttrs) {
        var label = tElement.find('label').text(tAttrs.label);
        var buttonset = tElement.find('buttonset');
        buttonset.attr({
          'data-name': tAttrs.model.replace(/\W+/, '_'),
          'data-ng-model': tAttrs.model,
          'data-values': tAttrs.values,
        });

        return function postLink(scope, iElement, iAttrs) {};
      },
    };
  });
