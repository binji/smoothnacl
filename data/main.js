/**
 * Copyright (c) 2012 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 **/
"use strict"

var updateTimeoutID = {
  'kernel': null,
  'smoother': null,
  'palette': null
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

function getUpdateGroupMessageFromValues(groupName, values) {
  return 'Set' + upperCaseFirst(groupName) + ':' + values.join(',');
}

function getUpdateGroupMessageFromUI(groupName) {
  return getUpdateGroupMessageFromValues(groupName, getGroupValuesFromUI(groupName));
}

function getUpdateMessagesFromPresetValues(values) {
  return {
    'kernel': getUpdateGroupMessageFromValues('kernel', values.slice(0, 3)),
    'smoother': getUpdateGroupMessageFromValues('smoother', values.slice(3, 14))
  };
}

function getGroupValuesFromUI(groupName) {
  return $.makeArray($('.' + groupName + ' .has-value').map(function () {
    return $(this).data('realValue')();
  }));
}

function getPresetValuesFromUI() {
  return getGroupValuesFromUI('kernel').concat(
      getGroupValuesFromUI('smoother'));
}

function updateGroupFromValues(groupName, values) {
  var msg = getUpdateGroupMessageFromValues(groupName, values);
  $('#nacl_module').get(0).postMessage(msg);
  updateTimeoutID[groupName] = null;
}

function updateGroupFromUI(groupName) {
  updateGroupFromValues(groupName, getGroupValuesFromUI(groupName));
}

function updateGroup(groupName) {
  if (groupName == 'palette')
    updateGradient();

  if (!updateTimeoutID[groupName]) {
    updateTimeoutID[groupName] =
        window.setTimeout(updateGroupFromUI, 200, groupName);
  }
}

function addPreset(index, name) {
  $('#presetMenu').append(
      $('<li>').addClass('setting-row')
               .append(
                   $('<a>').attr('href', '#')
                           .data('value', index)
                           .text(name)));
}

function loadPresets(array) {
  var menu = $('#presetMenu');
  for (var i = 0; i < array.length; ++i)
    addPreset(i, array[i][0]);
}

function initPresets() {
  var savedPresets = localStorage.getItem('presets');
  if (savedPresets) {
    loadPresets(JSON.parse(savedPresets));
  } else {
    loadPresets(presets);
  }

  $('#presetMenu').menu({
    select: function (e, ui) {
      var presetIndex = ui.item.children('a:first').data('value');
      onPresetChanged(presetIndex);
    }
  });
}

function savePreset(name, values) {
  presets.push([name, values]);
  localStorage.setItem('presets', JSON.stringify(presets));
  addPreset(presets.length - 1, name);
  $('#presetMenu').menu('refresh');
}

function updateUIWithValues(values) {
  $('.kernel .has-value').each(function (i) {
    $(this).data('updateValueForPreset')(values.slice(0, 3)[i]);
  });
  $('.smoother .has-value').each(function (i) {
    $(this).data('updateValueForPreset')(values.slice(3, 14)[i]);
  });
}

function onPresetChanged(index) {
  updateUIWithValues(presets[index][1]);
  // Skip the timeout, if the user wants to spam the button they can.
  updateGroupFromUI('kernel');
  updateGroupFromUI('smoother');
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

    optionEl.on('change', function (e) {
      updateGroup(group);
    });
    var labelEl = $('<label>').attr('for', optionId)
                              .text(values[i]);

    buttonsetEl.append(optionEl).append(labelEl);
  }
  buttonsetEl.buttonset();
  buttonsetEl.data({
    realValue: function () {
      return buttonsetEl.children('input:checked').data('value');
    },
    updateValueForPreset: function (value) {
      buttonsetEl.children('input').removeAttr('checked');
      buttonsetEl.children('input').eq(value).attr('checked', 'true');
      buttonsetEl.buttonset('refresh');
    }
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

  slider.on('slide', function (e, ui) {
    el.find('span').text((ui.value / mult).toFixed(prec));
    updateGroup(group);
  });
  slider.data({
    realValue: function () {
      return sliderWidget.value() / mult;
    },
    textValue: function () {
      return (sliderWidget.value() / mult).toFixed(prec);
    },
    updateValueForPreset: function (value) {
      sliderWidget.value(value * mult);
      el.find('span').text(value.toFixed(prec));
    }
  });
  return slider;
};

function updateGradient() {
  var repeating = $('.palette .buttonset .has-value').eq(0).data('realValue')();
  if (repeating)
    var gradient = '-webkit-repeating-linear-gradient(left';
  else
    var gradient = '-webkit-linear-gradient(left';
  $('.palette .color').each(function () {
    var color = $(this).find('input.has-value').eq(0).val();
    var stop = $(this).find('div.has-value').eq(0).data('textValue')();
    gradient += ', ' + color + ' ' + stop + '%';
  });

  gradient += ')';

  $('#gradient').css('background', gradient);
}

function makeColorstopSlider(group, el) {
  var slider = $('<div/>').addClass('has-value').slider({
    value: el.data('stop'),
    min: 0,
    max: 100,
    range: 'min',
  });
  var sliderWidget = slider.data('slider');

  slider.on('slide', function (e, ui) { updateGroup('palette'); });
  slider.data({
    realValue: function () { return sliderWidget.value(); },
    textValue: function () { return sliderWidget.value().toString(); }
  });
  return slider;
}

function makeColorUI(group, el) {
  var input = $('<input>').addClass('has-value')
      .attr('type', 'hidden')
      .val(el.data('value'))
      .data({ realValue: function () { return $(input).val(); } })
  var slider = makeColorstopSlider(group, el);
  el.addClass('color setting-row')
    .append($('<div/>').addClass('value')
        .append(input)
        .append(slider));
  input.miniColors({
    change: function (hex, rgba) { updateGroup(group); }
  });
}

function makeButtonsetUI(group, el) {
  var name = el.text();
  var buttonset = makeButtonset(group, el);
  el.empty().addClass('buttonset setting-row')
    .append($('<label/>').text(name))
    .append(buttonset.addClass('value'));
}

function makeRangeUI(group, el) {
  var name = el.text();
  var slider = makePrecSlider(group, el);
  el.empty().addClass('range setting-row')
    .append($('<label/>').text(name))
    .append($('<div/>').addClass('value')
        .append($('<span/>').text(slider.data('textValue')()))
        .append(slider));
}

var UILookup = {
  color: makeColorUI,
  buttonset: makeButtonsetUI,
  range: makeRangeUI
};

function makeUIElement(group, el) {
  UILookup[el.data('type')](group, el);
}

function makeUIForGroup(group) {
  $('.' + group + ' > div').each(function () {
      makeUIElement(group, $(this));
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
    active: 3
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
    savePreset(presetName, getPresetValuesFromUI());
  });

  var groups = ['kernel', 'smoother', 'palette'];
  for (var i = 0; i < groups.length; ++i)
    makeUIForGroup(groups[i]);

  $('#colors')
      .sortable({
        items: '.color',
      })
      .on('sortupdate', function (e, ui) { updateGroup('palette'); });
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

function getInitialValues() {
  if (window.location.hash) {
    try {
      var valueString = atob(window.location.hash.slice(1));
      return valueString.split(',');
    } catch (exc) {}
  }

  var initialPreset = 0;
  return presets[initialPreset][1];
}

function makeMainEmbed(groupMessages) {
  var listenerEl = document.getElementById('listener');
  makeEmbed(listenerEl, {
    id: 'nacl_module',
    width: $('.ui-layout-center').width(),
    height: $('.ui-layout-center').height(),
    msg1: groupMessages.kernel,
    msg2: groupMessages.smoother,
    msg3: groupMessages.palette,
    msg4: 'Clear:0',
    msg5: 'Splat',
    msg6: 'SetRunOptions:continuous',
    msg7: 'SetDrawOptions:simulation',
  });

  // Listen for messages from this embed -- they're only fps updates.
  listenerEl.addEventListener('message', function (e) {
    $('#fps').text(e.data);
  }, true);
}

$(document).ready(function (){
  initPresets();
  setupUI();
  var initialValues = getInitialValues();
  updateUIWithValues(initialValues);
  var messages = getUpdateMessagesFromPresetValues(initialValues);
  // TODO(binji): Should be getting these values from the presets...
  messages.palette = getUpdateGroupMessageFromUI('palette');
  makeMainEmbed(messages);
  updateGradient();
});
