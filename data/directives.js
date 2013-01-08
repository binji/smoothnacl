/**
 * Copyright (c) 2012 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 **/
"use strict";

(function () {

  var buttonsetDirective = function () {
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
  };

  var sliderDirective = function () {
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
  };

  var sliderRowDirective = function () {
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
          'ng-model': model,
          'data-min': tAttrs.min,
          'data-max': tAttrs.max,
          'data-prec': tAttrs.prec,
        });

        return function postLink(scope, iElement, iAttrs) {};
      },
    };
  };

  var buttonsetRowDirective = function () {
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
          'ng-model': tAttrs.model,
          'data-values': tAttrs.values,
        });

        return function postLink(scope, iElement, iAttrs) {};
      },
    };
  };

  var colorPickerDirective = function () {
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
  };

  var colorstopDirective = function () {
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
          '<div class="close-button-div" ng-click="removeColorstop($index)">' +
            '<span class="ui-icon ui-icon-closethick"/>' +
          '</div>' +
        '</div>',
      compile: function (tElement, tAttrs) {
        tElement.find('color-picker').attr('ng-model', tAttrs.model + '.color');
        tElement.find('slider').attr('ng-model', tAttrs.model + '.stop');

        return function postLink(scope, iElement, iAttrs) {};
      },
    };
  };

  var colorGradientDirective = function () {
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
  };

  var buttonDirective = function () {
    return {
      restrict: 'E',
      link: function (scope, iElement, iAttrs) {
        iElement.button();
      },
    };
  };

  var disableButtonDirective = function () {
    return {
      restrict: 'A',
      link: function (scope, iElement, iAttrs) {
        scope.$watch(iAttrs.disableButton, function (value) {
          iElement.button('option', 'disabled', value);
        });
      },
    };
  };

  var presetDirective = function () {
    return {
      restrict: 'E',
      replace: true,
      template:
        '<div class="preset-item">' +
          '<label>{{preset.name}}</label>' +
          '<img ng-src="{{preset.imgSrc}}">' +
          '<div class="close-button-div" ng-show="preset.canRemove"' +
              'ng-click="removePreset($index)">' +
            '<span class="ui-icon ui-icon-closethick"/>' +
          '</div>' +
        '</div>',
    };
  };

  var masonryDirective = ['$timeout', function ($timeout) {
    return function postLink(scope, iElement, iAttrs) {
      $timeout(function () {
        iElement.masonry({
          columnWidth: 128,
          gutterWidth: 16,
          isFitWidth: true,
          itemSelector: '.preset-item',
        });
      });

      var oldValue = null;
      scope.$watch(iAttrs.model, function (newValue) {
        // This watches too many changes. We only want to reload when the
        // presets array changes, not if any of the array values change.
        // (For some reason, passing false to $watch doesn't do this).
        if (oldValue !== null) {
          if (oldValue.length === newValue.length) {
            // Check to see if the objects of the array are equal, if so,
            // the only change that occurred was a change to an item's
            // value.
            var allMatch = true;
            for (var i = 0; i < oldValue.length; ++i) {
              if (oldValue[i] !== newValue[i]) {
                allMatch = false;
                break;
              }
            }

            if (allMatch) {
              return;
            }
          }
        }

        // Otherwise, reload masonry.
        iElement.masonry('reload');

        // Make oldValue a shallow copy of newValue.
        oldValue = [];
        for (var i = 0; i < newValue.length; ++i)
          oldValue.push(newValue[i]);
      }, true);  // true => compare using angular.equals
    };
  }];

  var naclModuleDirective = ['$interpolate', '$timeout', '$rootScope',
      function ($interpolate, $timeout, $rootScope) {
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
        scope.$on('clear', function () {
          embed[0].postMessage('Clear:0');
        });
        scope.$on('splat', function () {
          embed[0].postMessage('Splat');
        });

        scope.requestId = 0;
        scope.requestCallbacks = {};

        scope.$on('takeScreenshot', function (event, params, callback) {
          var requestId = scope.requestId++;
          scope.requestCallbacks[requestId] = callback;
          params = angular.copy(params)
          params.unshift(requestId)
          embed[0].postMessage('Screenshot:' + params.join(','))
        });

        // bind/on doesn't work for messages from NaCl module.
        iElement[0].addEventListener('message', function (e) {
          console.log('Got message from nacl module.');
          if (typeof(e.data) === 'string') {
            // Skip "FPS: "
            var fps = +e.data.substr(5);
            // FPS update.
            $rootScope.$apply(function (scope) {
              scope.fps = fps;
            });
          } else {
            var requestId = new Uint32Array(e.data, 0, 4)[0];
            console.log('Got screenshot with request id: ' + requestId);
            var imageData = new Uint8Array(e.data, 4);
            var blob = new Blob([imageData], {type: 'image/jpeg'});
            var url = webkitURL.createObjectURL(blob);
            var callback = scope.requestCallbacks[requestId];
            if (callback)
              callback(url);
            delete scope.requestCallbacks[requestId];
          }
        }, true);

        iElement.append(embed);
      }
    };
  }];

  angular.module('smoothnacl', [])
      .directive('button', buttonDirective)
      .directive('buttonset', buttonsetDirective)
      .directive('buttonsetRow', buttonsetRowDirective)
      .directive('colorGradient', colorGradientDirective)
      .directive('colorPicker', colorPickerDirective)
      .directive('colorstop', colorstopDirective)
      .directive('disableButton', disableButtonDirective)
      .directive('masonry', masonryDirective)
      .directive('naclModule', naclModuleDirective)
      .directive('preset', presetDirective)
      .directive('sliderRow', sliderRowDirective)
      .directive('slider', sliderDirective);

})();
