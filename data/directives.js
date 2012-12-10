/**
 * Copyright (c) 2012 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 **/
"use strict"

angular.module('directives', []).
  directive('buttonset', function () {
    return {
      require: '?ngModel',
      restrict: 'E',  // Element only.
      replace: true,
      template: '<div class="setting-row"></div>',
      link: function (scope, iElement, iAttrs, ngModel) {
        var name = iElement.data('name');
        var values = iAttrs.values.split(' ');
        var buttonsetEl = $('<div/>');
        for (var i = 0; i < values.length; ++i) {
          var valueText = values[i];
          var optionId = name.replace(' ', '_') + '_' + i;
          var labelEl = $('<label/>').attr('for', optionId)
                                     .text(valueText);
          var optionEl = $('<input/>').attr('type', 'radio')
                                      .attr('id', optionId)
                                      .attr('name', name)
                                      .data('value', i);
          buttonsetEl.append(labelEl).append(optionEl);
        }

        buttonsetEl.buttonset().addClass('value');
        iElement.append($('<label/>').text(name));
        iElement.append(buttonsetEl);

        if (ngModel) {
          ngModel.$render = function () {
            var value = ngModel.$viewValue;
            buttonsetEl.children('input').removeAttr('checked');
            buttonsetEl.children('input').eq(value).attr('checked', 'checked');
            buttonsetEl.buttonset('refresh');
          };

          buttonsetEl.bind('change', function (e) {
            scope.$apply(function () {
              var value = buttonsetEl.children('input:checked').data('value');
              ngModel.$setViewValue(value);
            });
          });
        }
      },
    };
  });
