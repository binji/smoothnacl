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
        var name = iAttrs.name.replace(/\W+/, '_');
        var values = iAttrs.values.split(' ');
        for (var i = 0; i < values.length; ++i) {
          var valueText = values[i];
          var optionId = name + i;
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
      template: '<div></div>',
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
          '<buttonset class="value">' +
          '</buttonset>' +
        '</div>',
      compile: function (tElement, tAttrs) {
        if (tAttrs.label)
          tElement.prepend($('<label/>').text(tAttrs.label));

        var buttonset = tElement.find('buttonset');
        buttonset.attr({
          'data-name': tAttrs.model.replace(/\W+/, '_'),
          'data-ng-model': tAttrs.model,
          'data-values': tAttrs.values,
        });

        return function postLink(scope, iElement, iAttrs) {};
      },
    };
  })
  .directive('colorPicker', function () {
    return {
      restrict: 'E',
      replace: true,
      require: 'ngModel',
      // We have to wrap the colorpicker input in a div -- miniColors adds a
      // span as sibling to the input for the colorpicker which screws up the
      // angular linking phase.
      template: '<div><input type="hidden"></div>',
      link: function (scope, iElement, iAttrs, ngModel) {
        var inputEl = iElement.find('input');
        // miniColors fires a change callback both from user input and when it
        // is changed programmatically. Ignore changes that occur when setting
        // the miniColors value.
        var isSettingValue = false;

        inputEl.miniColors({
          change: function (hex, rgba) {
            if (!isSettingValue) {
              scope.$apply(function () {
                ngModel.$setViewValue(hex);
              });
            }
          }
        });

        ngModel.$render = function () {
          isSettingValue = true;
          inputEl.miniColors('value', ngModel.$viewValue);
          isSettingValue = false;
        };
      },
    };
  })
  .directive('colorstop', function () {
    return {
      restrict: 'E',
      replace: true,
      template:
        '<div class="color setting-row">' +
          '<span class="handle ui-icon ui-icon-grip-dotted-vertical"/>' +
          '<div class="value">' +
            '<color-picker></color-picker>' +
            '<slider data-min="0" data-max="100" data-prec="0"></slider>' +
          '</div>' +
          '<div class="close-button-div" data-ng-click="removeColorstop($index)">' +
            '<span class="ui-icon ui-icon-closethick"/>' +
          '</div>' +
        '</div>',
      compile: function (tElement, tAttrs) {
        tElement.find('color-picker').attr('data-ng-model', tAttrs.model + '.color');
        tElement.find('slider').attr('data-ng-model', tAttrs.model + '.stop');

        return function postLink(scope, iElement, iAttrs) {};
      },
    };
  })
  .directive('colorGradient', function () {
    return {
      restrict: 'E',
      replace: true,
      template: '<div></div>',
      link: function (scope, iElement, iAttrs) {
        scope.$watch(iAttrs.model, function (newValue) {
          if (newValue.gradientType !== 0)
            var gradient = '-webkit-repeating-linear-gradient(left';
          else
            var gradient = '-webkit-linear-gradient(left';

          angular.forEach(newValue.colorstops, function (colorstop) {
            gradient += ', ' + colorstop.color + ' ' + colorstop.stop + '%';
          });
          gradient += ')';

          iElement.css('background', gradient);
        }, true);  // true => compare using angular.equals
      },
    };
  })
  .directive('button', function () {
    return {
      restrict: 'E',
      link: function (scope, iElement, iAttrs) {
        iElement.button();
      },
    };
  })
  .directive('preset', function () {
    return {
      restrict: 'E',
      replace: true,
      template:
        '<div class="preset-item">' +
          '<label>{{preset.name}}</label>' +
          '<img data-ng-src="{{preset.imgSrc}}">' +
        '</div>',
    };
  })
  .directive('masonry', function ($timeout) {
    return function postLink(scope, iElement, iAttrs) {
      $timeout(function () {
        iElement.masonry({
          columnWidth: 128,
          gutterWidth: 16,
          isFitWidth: true,
          itemSelector: '.preset-item',
        });
      });
    };
  })
  .directive('naclModule', function ($interpolate, $timeout) {
    return {
      restrict: 'A',
      link: function (scope, iElement, iAttrs) {
        var embed = $('<embed>');
        var attrs = {};

        angular.extend(attrs, {
          id: 'nacl_module',
          src: 'smoothlife.nmf',
          type: 'application/x-nacl',
        });

        // For some reason, the values are not correctly interpolated in
        // iAttrs. The pre-interpolated attrs are in iElement[0].attributes,
        // though, so we can redo the work (correctly).
        angular.forEach(iElement[0].attributes, function (attr) {
          if (attr.name.lastIndexOf('msg', 0) === 0) {
            attrs[attr.name] = $interpolate(attr.value)(scope);
          }
        });

        embed.attr(attrs);

        $timeout(function () {
          embed.attr({
            width: $('.ui-layout-center').width(),
            height: $('.ui-layout-center').height()
          });
        });

        scope.$watch('kernel', function () {
          embed[0].postMessage(scope.getKernelMessage());
        }, true);
        scope.$watch('smoother', function () {
          embed[0].postMessage(scope.getSmootherMessage());
        }, true);
        scope.$watch('palette', function () {
          embed[0].postMessage(scope.getPaletteMessage());
        }, true);
        scope.$on('clearSplat', function () {
          embed[0].postMessage('Clear:0');
          embed[0].postMessage('Splat');
        });

        iElement.append(embed);
      }
    };
  });
