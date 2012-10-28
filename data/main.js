var updateTimeoutID = {
  'kernel': null,
  'smoother': null
};
var presets = [
  ['Jellyfish', [4.0, 12.0, 1.0, 2, 0.115, 0.269, 0.523, 0.340, 0.746, 3, 4, 4, 0.028, 0.147]],
  ['Electric Gliders', [4.0, 12.0, 1.0, 0, 0.100, 0.278, 0.267, 0.365, 0.445, 3, 4, 4, 0.028, 0.147]],
  ['Worms and Donuts', [10.6, 31.8, 1.0, 1, 0.157, 0.092, 0.256, 0.098, 0.607, 3, 4, 4, 0.015, 0.340]],
  ['Collapsing Tunnels', [7.26, 21.8, 1.0, 1, 0.157, 0.192, 0.355, 0.200, 0.600, 3, 4, 4, 0.025, 0.490]],
  ['Growing Tube', [7.77, 27.2, 2.64, 3, 0.138, 0.666, 0.056, 0.175, 0.838, 2, 0, 2, 0.132, 0.311]]
];

function upperCaseFirst(s) {
  return s.charAt(0).toUpperCase() + s.substr(1);
}

function getUpdateGroupMessage(groupName) {
  return 'Set' + upperCaseFirst(groupName) + ':' +
      getGroupValuesString(groupName);
}

function getGroupValuesString(groupName) {
  var values = $('.' + groupName + ' .range > div').map(function () {
    return $(this).data('realValue')();
  });
  return Array.prototype.join.call(values, ',');
}

function getGroupValues(groupName) {
  return $.makeArray($('.' + groupName + ' .range > div').map(function () {
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
}

function updateGroup(groupName) {
  if (!updateTimeoutID[groupName]) {
    updateTimeoutID[groupName] =
        window.setTimeout(updateGroups, 200, groupName);
  }
}

function initPresets() {
  var menu = $('#presetMenu');
  for (var i = 0; i < presets.length; ++i) {
    var presetName = presets[i][0];
    menu.append('<li><a href="#" ' +
        'data-value="' + i + '">' + presetName + '</a></li>');
  }
  menu.menu({
    select: function (e, ui) {
      var presetIndex = ui.item.children('a:first').data('value');
      onPresetChanged(presetIndex);
    }
  });
}

function addPreset(name, values) {
  var menu = $('#presetMenu');
  presets.push([name, values]);
  menu.append('<li><a href="#" data-value="'+
      (presets.length-1)+'">'+name+'</a></li>');
  menu.menu('refresh');
}

function onPresetChanged(index) {
  var preset = presets[index];
  $('.kernel .range > div').each(function (i) {
    $(this).data('updatValueForPreset')(preset[1].slice(0, 3)[i]);
  });
  $('.smoother .range > div').each(function (i) {
    $(this).data('updatValueForPreset')(preset[1].slice(3, 14)[i]);
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

function makeValueSlider(group, el) {
  var values = el.data('values').split(' ');
  var slider = $('<div/>').slider({
    value: el.data('value'),
    min: 0,
    max: values.length - 1,
    range: 'min',
  });

  var sliderWidget = slider.data('slider');

  slider.bind('slide', function (e, ui) {
    el.children('span').text(values[ui.value]);
    updateGroup(group);
  });
  slider.data('realValue', function () {
    return sliderWidget.value();
  });
  slider.data('textValue', function () {
    return values[sliderWidget.value()];
  });
  slider.data('updatValueForPreset', function (value) {
    sliderWidget.value(value);
    el.children('span').text(values[value]);
  });
  return slider;
};

function makePrecSlider(group, el) {
  var prec = el.data('prec');
  var mult = Math.pow(10, prec);
  var slider = $('<div/>').slider({
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
  slider.data('updatValueForPreset', function (value) {
    sliderWidget.value(value * mult);
    el.children('span').text(value.toFixed(prec));
  });
  return slider;
};

function makeSlidersForGroup(group) {
  $('.' + group + ' > div').each(function () {
    var el = $(this);
    var slider;

    if (el.data('values'))
      slider = makeValueSlider(group, el);
    else
      slider = makePrecSlider(group, el);

    var name = el.text();
    el.empty().addClass('range');
    el.append('<label>' + name + '</label>');
    el.append('<span>' + slider.data('textValue')() + '</span>');
    el.append(slider);
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
    fillSpace: true,
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
    makeSlidersForGroup(groups[i]);
  }
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

function makeMainEmbed() {
  var kernelMessage;
  var smootherMessage;

  if (window.location.hash) {
    try {
      var valueString = atob(window.location.hash.slice(1));
      var values = valueString.split(',');
      kernelValues = 'SetKernel:'+values.slice(0, 3).join(',');
      smootherValues = 'SetSmoother:'+values.slice(3, 14).join(',');
    } catch (exc) {
      kernelValues = getUpdateGroupMessage('kernel');
      smootherValues = getUpdateGroupMessage('smoother');
    }
  } else {
    kernelValues = getUpdateGroupMessage('kernel');
    smootherValues = getUpdateGroupMessage('smoother');
  }

  var listenerEl = document.getElementById('listener');
  makeEmbed(listenerEl, {
    id: 'nacl_module',
    width: $('.ui-layout-center').width(),
    height: $('.ui-layout-center').height(),
    msg1: kernelValues,
    msg2: smootherValues,
    msg3: 'Clear:0',
    msg4: 'Splat',
    msg5: 'SetRunOptions:continuous',
    msg6: 'SetDrawOptions:simulation',
  });

  // Listen for messages from this embed -- they're only fps updates.
  listenerEl.addEventListener('message', function (e) {
    $('#fps').text(e.data);
  }, true);
}

$(document).ready(function (){
  initPresets();
  setupUI();
  makeMainEmbed();
});
