/**
 * Copyright (c) 2012 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 **/

var updateTimeoutID = {
  'kernel': null,
  'smoother': null
};
var presets = [
  ['Jellyfish', [4.0, 12.0, 1.0, 2, 0.115, 0.269, 0.523, 0.340, 0.746, 3, 4, 4, 0.028, 0.147]],
  ['Electric Gliders', [4.0, 12.0, 1.0, 0, 0.100, 0.278, 0.267, 0.365, 0.445, 3, 4, 4, 0.028, 0.147]],
  ['Worms and Donuts', [10.6, 31.8, 1.0, 1, 0.157, 0.092, 0.256, 0.098, 0.607, 3, 4, 4, 0.015, 0.340]],
  ['Collapsing Tunnels', [7.26, 21.8, 1.0, 1, 0.157, 0.192, 0.355, 0.200, 0.600, 3, 4, 4, 0.025, 0.490]],
  ['Growing Tube', [7.77, 27.2, 2.64, 3, 0.138, 0.666, 0.056, 0.175, 0.838, 2, 0, 2, 0.132, 0.311]],
  ['Stitches \'n\' Jitters', [10.4, 12.0, 1, 2, 0.182, 0.27, 0.508, 0.35, 0.749, 2, 3, 4, 0.028, 0.147]]
];

function upperCaseFirst(s) {
  return s.charAt(0).toUpperCase() + s.substr(1);
}

function getUpdateGroupMessage(groupName) {
  return 'Set' + upperCaseFirst(groupName) + ':' +
      getGroupValuesString(groupName);
}

function getGroupValuesString(groupName) {
  var values = $('.' + groupName + ' .has-value').map(function () {
    return $(this).data('realValue')();
  });
  return Array.prototype.join.call(values, ',');
}

function getGroupValues(groupName) {
  return $.makeArray($('.' + groupName + ' .has-value').map(function () {
    return $(this).data('realValue')();
  }));
}

function getValues() {
  return getGroupValues('kernel').concat(getGroupValues('smoother'));
}

function getValuesString() {
  return Array.prototype.join.call(getValues(), ',');
}


function updateGroups(groupName) {
  var msg = getUpdateGroupMessage(groupName);
  $('#nacl_module').get(0).postMessage(msg);
  updateTimeoutID[groupName] = null;

  /*
  if (groupName == 'smoother') {
    $('#smoother_module').get(0).postMessage(msg);
    $('#smoother_module').get(0).postMessage('SetRunOptions:none');
  }
  */
}

function updateGroup(groupName) {
  if (!updateTimeoutID[groupName]) {
    updateTimeoutID[groupName] =
        window.setTimeout(updateGroups, 200, groupName);
  }
}

function loadPresets(array) {
  var menu = $('#presetMenu');
  for (var i = 0; i < array.length; ++i) {
    var presetName = array[i][0];
    menu.append(
        $('<li>').append(
            $('<a>').attr('href', '#')
                    .data('value', i)
                    .text(presetName)));
  }
}

function initPresets() {
  var savedPresets = localStorage.getItem('presets');
  if (savedPresets) {
    loadPresets(JSON.parse(savedPresets));
  } else {
    loadPresets(presets);
  }

  var menu = $('#presetMenu');
  menu.menu({
    select: function (e, ui) {
      var presetIndex = ui.item.children('a:first').data('value');
      onPresetChanged(presetIndex);
    }
  });
}

function addPreset(name, values) {
  presets.push([name, values]);
  localStorage.setItem('presets', JSON.stringify(presets));

  var menu = $('#presetMenu');
  menu.append(
      $('<li>').append(
          $('<a>').attr('href', '#')
                  .data('value', presets.length - 1)
                  .text(name)));
  menu.menu('refresh');
}

function onPresetChanged(index) {
  var preset = presets[index];
  $('.kernel .has-value').each(function (i) {
    $(this).data('updateValueForPreset')(preset[1].slice(0, 3)[i]);
  });
  $('.smoother .has-value').each(function (i) {
    $(this).data('updateValueForPreset')(preset[1].slice(3, 14)[i]);
  });
  // Skip the timeout, if the user wants to spam the button they can.
  updateGroups('kernel');
  updateGroups('smoother');
  var module = $('#nacl_module').get(0);
  // When changing to a preset, reset the screen to make sure it looks
  // interesting.
  module.postMessage('Clear:0');
  module.postMessage('Splat');
}

function makeButtonset(group, el) {
  var values = el.data('values').split(' ');
  var buttonsetEl = $('<div/>').addClass('has-value');
  for (var i = 0; i < values.length; ++i) {
    var optionId = el.text().replace(' ', '_') + '_' + i;
    var optionEl = $('<input>').attr('type', 'radio')
                               .attr('id', optionId)
                               .attr('name', el.text())
                               .data('value', i);
    if (i === el.data('value'))
      optionEl.attr('checked', 'true');

    optionEl.bind('change', function (e) {
      updateGroup(group);
    });
    var labelEl = $('<label for="'+optionId+'"/>').text(values[i]);

    buttonsetEl.append(optionEl).append(labelEl);
  }
  buttonsetEl.buttonset();
  buttonsetEl.data('realValue', function () {
    return buttonsetEl.children('input:checked').data('value');
  });
  buttonsetEl.data('updateValueForPreset', function (value) {
    buttonsetEl.children('input').removeAttr('checked');
    buttonsetEl.children('input').eq(value).attr('checked', 'true');
    buttonsetEl.buttonset('refresh');
  });

  return buttonsetEl;
};

function makePrecSlider(group, el) {
  var prec = el.data('prec');
  var mult = Math.pow(10, prec);
  var slider = $('<div/>').addClass('has-value').slider({
    value: el.data('value') * mult,
    min: el.data('min') * mult,
    max: el.data('max') * mult,
    range: 'min',
  });
  var sliderWidget = slider.data('slider');

  slider.bind('slide', function (e, ui) {
    el.children('span').text((ui.value / mult).toFixed(prec));
    updateGroup(group);
  });
  slider.data('realValue', function () {
    return sliderWidget.value() / mult;
  });
  slider.data('textValue', function () {
    return (sliderWidget.value() / mult).toFixed(prec);
  });
  slider.data('updateValueForPreset', function (value) {
    sliderWidget.value(value * mult);
    el.children('span').text(value.toFixed(prec));
  });
  return slider;
};

function makeUIForGroup(group) {
  $('.' + group + ' > div').each(function () {
    var el = $(this);

    if (el.data('values')) {
      var buttonset = makeButtonset(group, el);
      var name = el.text();
      el.empty().addClass('buttonset');
      el.append('<label>' + name + '</label>');
      el.append(buttonset);
      el.append($('<div>').addClass('clearfix'));
    } else {
      var slider = makePrecSlider(group, el);
      var name = el.text();
      el.empty().addClass('range');
      el.append('<label>' + name + '</label>');
      el.append('<span>' + slider.data('textValue')() + '</span>');
      el.append(slider);
    }

  });
};

function setupUI() {
  $('document').tooltip();
  $('body').layout({
    slidable: false,
    center__onresize: function (name, el) {
      if (!document.webkitIsFullScreen) {
        $('#nacl_module').attr('width', el.width()).attr('height', el.height());
      }
    },
    north__closable: false,
    north__resizable: false,
    north__spacing_open: 2,
    west__minSize: 400,
    west__onresize: function () {
      $('.accordion').accordion('resize');
    },
    west__resizable: false,
    livePaneResizing: true
  });

  $('.accordion').accordion({
    collapsible: true,
    fillSpace: true,
    active: 2
  });

  $('#clear').button().click(function (e) {
    $('#nacl_module').get(0).postMessage('Clear:0');
  });
  $('#splat').button().click(function (e) {
    $('#nacl_module').get(0).postMessage('Splat');
  });
  $('#fullscreen').button().click(function (e) {
    $('#nacl_module').get(0).postMessage('SetFullscreen:true');
  });
  $('#savePresetButton').button().click(function (e) {
    var presetName = $('#savePresetName').val();
    addPreset(presetName, getValues());
  });

  var groups = ['kernel', 'smoother'];
  for (var i = 0; i < groups.length; ++i) {
    makeUIForGroup(groups[i]);
  }

  $('.color-picker').iris({
    hide: false,
    palettes: true
  });
}

function makeEmbed(appendChildTo, attrs) {
  // Generate embed using current sizes and values.
  var embedEl = document.createElement('embed');
  attrs.src = 'smoothlife.nmf';
  attrs.type = 'application/x-nacl';
  for (var k in attrs) {
    embedEl.setAttribute(k, attrs[k]);
  }
  appendChildTo.appendChild(embedEl);
}

function getInitialMessages() {
  var kernelMessage;
  var smootherMessage;

  if (window.location.hash) {
    try {
      var valueString = atob(window.location.hash.slice(1));
      var values = valueString.split(',');
      kernelMessage = 'SetKernel:'+values.slice(0, 3).join(',');
      smootherMessage = 'SetSmoother:'+values.slice(3, 14).join(',');
    } catch (exc) {
      kernelMessage = getUpdateGroupMessage('kernel');
      smootherMessage = getUpdateGroupMessage('smoother');
    }
  } else {
    kernelMessage = getUpdateGroupMessage('kernel');
    smootherMessage = getUpdateGroupMessage('smoother');
  }

  return {
    'kernel': kernelMessage,
    'smoother': smootherMessage
  };
}

function makeMainEmbed(groupMessages) {
  var listenerEl = document.getElementById('listener');
  makeEmbed(listenerEl, {
    id: 'nacl_module',
    width: $('.ui-layout-center').width(),
    height: $('.ui-layout-center').height(),
    msg1: groupMessages.kernel,
    msg2: groupMessages.smoother,
    msg3: 'Clear:0',
    msg4: 'Splat',
    msg5: 'SetRunOptions:continuous',
    msg6: 'SetDrawOptions:simulation',
//    msg6: 'SetDrawOptions:palette',
    msg7: 'SetPalette',
  });

  // Listen for messages from this embed -- they're only fps updates.
  listenerEl.addEventListener('message', function (e) {
    $('#fps').text(e.data);
  }, true);
}

function makeSmootherEmbed(groupMessages) {
  var smootherEmbed = document.getElementById('smootherEmbed');
  makeEmbed(smootherEmbed, {
    id: 'smoother_module',
    width: 150,
    height: 150,
    msg1: groupMessages.smoother,
    msg2: 'Clear:0',
    msg3: 'SetRunOptions:none',
    msg4: 'SetDrawOptions:smoother'
  });
}

$(document).ready(function (){
  initPresets();
  setupUI();
  var groupMessages = getInitialMessages();
  makeMainEmbed(groupMessages);
  //makeSmootherEmbed(groupMessages);
});
