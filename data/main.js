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
  ["Fire Jellies",[4,12,1],[2,0.115,0.269,0.523,0.34,0.746,3,4,4,0.028,0.147],[0,"#36065e",0,"#c24242",77,"#8a19b0",91,"#ff9900",99,"#f5c816",99]],
  ["Magical Maze",[4,12,1],[3,0.12,0.218,0.267,0.365,0.445,3,4,4,0.028,0.147],[0,"#000000",0,"#0f8a84",38,"#f5f5c1",43,"#158a34",70,"#89e681",100]],
  ["Waterbug Gliders",[4,12,1],[0,0.09,0.276,0.27,0.365,0.445,1,4,4,0.028,0.147],[0,"#93afd9",11,"#9cf0ff",92,"#edfdff",100]],
  ["Stitches 'n' Jitters",[10.4,12,1],[2,0.082,0.302,0.481,0.35,0.749,2,3,4,0.028,0.147],[0,"#000000",11,"#ffffff",22,"#19a68a",85,"#6b0808",98]],
  ["Blink Tubes",[7.8,27.2,2.6],[3,0.21,0.714,0.056,0.175,0.838,2,0,2,0.132,0.311],[0,"#0a1340",0,"#ffffff",55,"#4da8a3",83,"#2652ab",99,"#2f1e75",46]],
  ["Oil Slick",[4,12,1],[2,0.115,0.269,0.496,0.34,0.767,3,4,4,0.028,0.147],[0,"#b8cfcf",0,"#3f5a5c",77,"#1a330a",91,"#c0e0dc",99]],
  ["Worms",[10.6,31.8,1],[1,0.157,0.092,0.256,0.098,0.607,3,4,4,0.015,0.34],[0,"#4d3e3e",0,"#9a1ac9",77,"#aaf09e",100]],
];

function upperCaseFirst(s) {
  return s.charAt(0).toUpperCase() + s.substr(1);
}

function dashesToCamelCase(s) {
  var result = '';
  var pos = 0;
  var dash = s.indexOf('-');
  while (dash !== -1) {
    result += upperCaseFirst(s.slice(pos, dash));
    pos = dash + 1;
    dash = s.indexOf('-', pos + 1);
  }

  result += upperCaseFirst(s.slice(pos));
  return result;
}

function getUpdateGroupMessageFromValues(groupName, values) {
  return 'Set' + dashesToCamelCase(groupName) + ':' + values.join(',');
}

function getUpdateGroupMessageFromUI(groupName) {
  return getUpdateGroupMessageFromValues(
      groupName,
      getGroupValuesFromUI(groupName));
}

function getUpdateMessagesFromPresetValues(values) {
  return {
    'kernel': getUpdateGroupMessageFromValues('kernel', values[0]),
    'smoother': getUpdateGroupMessageFromValues('smoother', values[1]),
    'palette': getUpdateGroupMessageFromValues('palette', values[2])
  };
}

function getGroupValuesFromUI(groupName) {
  return $.makeArray($('.' + groupName + ' .has-value').map(function () {
    return $(this).data('realValue')();
  }));
}

function getPresetValuesFromUI() {
  return [
    getGroupValuesFromUI('kernel'),
    getGroupValuesFromUI('smoother'),
    getGroupValuesFromUI('palette')
  ]
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

function makePresetElement(name, values, isUserPreset) {
  var itemEl =
      $('<li/>').addClass('setting-row')
                .append(
          $('<a/>').attr('href', '#')
                   .data('value', JSON.stringify(values))
                   .text(name));

  if (isUserPreset) {
    itemEl.addClass('user-preset')
          .append(
        $('<div/>').addClass('close-button-div')
                   .click(function (e) {
                     itemEl.hide('blind', 200, function () {
                       itemEl.remove();
                       savePresetsToLocalStorage();
                     });
                     e.stopPropagation();
                   })
                   .append(
            $('<span/>').addClass('ui-icon ui-icon-closethick')));
  }

  return itemEl;
}

function loadPresets(array, isUserPreset) {
  var menu = $('#presetMenu');
  for (var i = 0; i < array.length; ++i) {
    var presetEl = makePresetElement(
        array[i][0],
        array[i].slice(1, 4),
        isUserPreset);
    menu.append(presetEl);
  }
}

function initPresets() {
  loadPresets(presets, false);
  $('#presetMenu').menu({
    select: function (e, ui) {
      var presetValuesJson = ui.item.children('a:first').data('value');
      onPresetChanged(JSON.parse(presetValuesJson));
    }
  });

  if (chrome !== undefined && chrome.storage !== undefined) {
    chrome.storage.local.get('presets', function (items) {
      loadPresets(JSON.parse(items['presets']), true);
      $('#presetMenu').menu('refresh');
    });
  } else if (window.localStorage !== undefined) {
    var savedPresets = window.localStorage.getItem('presets');
    if (savedPresets) {
      loadPresets(JSON.parse(savedPresets), true);
      $('#presetMenu').menu('refresh');
    }
  }
}

function savePreset(name, values) {
  var presetEl = makePresetElement(name, values, true);
  $('#presetMenu').prepend(presetEl)
                  .menu('refresh');
  presetEl.hide();
  presetEl.show('blind', 200, function () {
    savePresetsToLocalStorage();
  });
}

function savePresetsToLocalStorage() {
  var presetValues = [];
  $('#presetMenu > li.user-preset > a').each(function () {
    var preset = [$(this).text()];
    preset = preset.concat(JSON.parse($(this).data('value')));
    presetValues.push(preset);
  });
  var presetValuesJson = JSON.stringify(presetValues);
  if (chrome !== undefined && chrome.storage !== undefined)
    chrome.storage.local.set({'presets': presetValuesJson});
  else if (window.localStorage !== undefined)
    window.localStorage.setItem('presets', presetValuesJson);

  $('#presetTextarea').text(presetValuesJson);
}

function updateUIWithValues(values) {
  $('.kernel .has-value').each(function (i) {
    $(this).data('updateValueForPreset')(values[0][i]);
  });
  $('.smoother .has-value').each(function (i) {
    $(this).data('updateValueForPreset')(values[1][i]);
  });

  $('.palette .color').remove();
  $('.palette .buttonset .has-value')
      .data('updateValueForPreset')(values[2][0]);
  for (var i = 1; i < values[2].length; i += 2) {
    addColorstopUI(values[2][i + 0], values[2][i + 1]);
  }
  updateGradient();
}

function onPresetChanged(values) {
  updateUIWithValues(values);
  // Skip the timeout, if the user wants to spam the button they can.
  updateGroupFromUI('kernel');
  updateGroupFromUI('smoother');
  updateGroupFromUI('palette');
  var module = $('#nacl_module').get(0);
  // When changing to a preset, reset the screen to make sure it looks
  // interesting.
  module.postMessage('Clear:0');
  module.postMessage('Splat');
}

function makeButtonset(group, el) {
  var values = el.data('values').split(' ');
  var valueType = el.data('valueType');
  var buttonsetEl = $('<div/>').addClass('has-value');
  for (var i = 0; i < values.length; ++i) {
    var optionId = el.text().replace(' ', '_') + '_' + i;
    var optionEl = $('<input>').attr('type', 'radio')
                               .attr('id', optionId)
                               .attr('name', el.text())
                               .data('value', i);
    if (valueType === 'string' && values[i] === el.data('value'))
      optionEl.attr('checked', 'true');
    else if (i === el.data('value'))
      optionEl.attr('checked', 'true');

    optionEl.on('change', function (e) {
      updateGroup(group);
    });
    var labelEl = $('<label/>').attr('for', optionId)
                               .text(values[i]);

    buttonsetEl.append(optionEl).append(labelEl);
  }
  buttonsetEl.buttonset();
  buttonsetEl.data({
    realValue: function () {
      var index = buttonsetEl.children('input:checked').data('value');
      return valueType === 'string' ? values[index] : index;
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

function makeColorstopSlider(stop) {
  var slider = $('<div/>').addClass('has-value').slider({
    value: stop,
    min: 0,
    max: 100,
    range: 'min',
  });
  var sliderWidget = slider.data('slider');

  slider.on('slide', function (e, ui) { updateGroup('palette'); });
  slider.data({
    realValue: function () { return sliderWidget.value(); },
    textValue: function () { return sliderWidget.value().toString(); },
    updateValueForPreset: function (value) {
      sliderWidget.value(value);
    }
  });
  return slider;
}

function makeColorstopUI(color, stop) {
  var input =
      $('<input>').addClass('has-value')
                  .attr('type', 'hidden')
                  .val(color)
                  .data({
                    realValue: function () { return $(input).val(); },
                    updateValueForPreset: function (value) {
                      $(input).val(value);
                    }
                  });
  var slider = makeColorstopSlider(stop);
  var ui =
      $('<div/>').addClass('color setting-row')
                 .append(
          $('<span/>').addClass('handle')
                      .addClass('ui-icon ui-icon-grip-dotted-vertical'))
                 .append(
          $('<div/>').addClass('value')
                     .append(input)
                     .append(slider))
                 .append(
          $('<div/>').addClass('close-button-div')
                     .click(function (e) {
                       $(ui).hide('blind', 200, function () {
                         $(ui).remove();
                         updateGroup('palette');
                       });
                       e.stopPropagation();
                     })
                     .append(
              $('<span/>').addClass('ui-icon ui-icon-closethick')));
  input.miniColors({
    change: function (hex, rgba) { updateGroup('palette'); }
  });

  return ui;
}

function addColorstopUI(color, stop, animateCallback) {
  var ui = makeColorstopUI(color, stop);
  $('#colors').append(ui);
  if (animateCallback) {
    ui.hide();
    ui.show('blind', 200, animateCallback);
  }
}

function makeButtonsetUI(group, el) {
  var name = el.text();
  var buttonset = makeButtonset(group, el);
  el.empty().addClass('buttonset setting-row');

  if (!el.data('noLabel'))
    el.append($('<label/>').text(name));

  el.append(buttonset.addClass('value'));
}

function makeRangeUI(group, el) {
  var name = el.text();
  var slider = makePrecSlider(group, el);
  el.empty().addClass('range setting-row')
            .append(
    $('<label/>').text(name))
            .append(
    $('<div/>').addClass('value')
               .append(
        $('<span/>').text(slider.data('textValue')()))
               .append(
        slider));
}

var UILookup = {
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
  $(document).tooltip();
  $('body').layout({
    slidable: false,
    center__onresize: function (name, el) {
      if (!document.webkitIsFullScreen) {
        $('#nacl_module').attr('width', el.width()).attr('height', el.height());
      }
    },
    east__minSize: 400,
    east__resizable: true,
    east__onresize: function () {
      $('.tabs').tabs('refresh');
    },
    livePaneResizing: true
  });

  $('.tabs').tabs({
    active: 1,
    heightStyle: 'fill',
  });

  $('#clear').button().click(function (e) {
    $('#nacl_module').get(0).postMessage('Clear:0');
  });
  $('#splat').button().click(function (e) {
    $('#nacl_module').get(0).postMessage('Splat');
  });
  /*
  $('#fullscreen').button().click(function (e) {
    $('#nacl_module').get(0).postMessage('SetFullscreen:true');
  });
  */
  $('#savePresetButton').button().click(function (e) {
    var presetName = $('#savePresetName').val();
    savePreset(presetName, getPresetValuesFromUI());
    $('#savePresetName').val('');
    return false;
  });

  var groups = ['kernel', 'smoother', 'palette', 'draw-options'];
  for (var i = 0; i < groups.length; ++i)
    makeUIForGroup(groups[i]);

  // Add plus button.
  /*
  $('#gradientType').prepend(
      $('<div/>').addClass('plus-button-div')
                 .append(
          $('<span/>').addClass('ui-icon ui-icon-plusthick')));
          */

  $('.plus-button-div').button().click(function () {
    addColorstopUI('#ffffff', 100, function () {
      updateGroup('palette');
    });
  });

  $('#colors')
      .sortable({ items: '.color', handle: 'span.handle' })
      .on('sortupdate', function (e, ui) { updateGroup('palette'); });

  var buttonBarTimeout;
  $('#buttonBar').hide()
                 .mouseover(function (e) {
                   window.clearTimeout(buttonBarTimeout);
                   buttonBarTimeout = 0;
                   e.stopPropagation();
                 });
  $('#listener').mousemove(function (e) {
    $('#buttonBar').show('blind', 200, function () {
      if (buttonBarTimeout)
        window.clearTimeout(buttonBarTimeout);

      buttonBarTimeout = window.setTimeout(function () {
        $('#buttonBar').hide('blind', 200);
        buttonBarTimeout = 0;
      }, 1000);
    });
  });

  $('#listener').mousedown(function (e) {
    $('#nacl_module').get(0).postMessage('SetRunOptions:noSimulation,run');
    e.stopPropagation();
  }).mouseup(function (e) {
    $('#nacl_module').get(0).postMessage('SetRunOptions:simulation,run');
    e.stopPropagation();
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

function getInitialValues() {
  if (window.location.hash) {
    try {
      var valueString = atob(window.location.hash.slice(1));
      return valueString.split(',');
    } catch (exc) {}
  }

  var initialPreset = 0;
  return presets[initialPreset].slice(1, 4);
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
    msg6: 'SetRunOptions:simulation,run',
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
  makeMainEmbed(messages);
  updateGradient();
});
