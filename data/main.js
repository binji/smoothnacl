var updateTimeoutID = {
  'kernel': null,
  'smoother': null
};
var presets = [
  ['preset1', [[12.0, 3.0, 12.0], [2, 0.115, 0.269, 0.523, 0.340, 0.746, 3, 4, 4, 0.028, 0.147]]],
  ['preset2', [[12.0, 3.0, 12.0], [0, 0.100, 0.278, 0.267, 0.365, 0.445, 3, 4, 4, 0.028, 0.147]]],
  ['preset3', [[31.8, 3.0, 31.8], [1, 0.157, 0.092, 0.256, 0.098, 0.607, 3, 4, 4, 0.015, 0.340]]],
  ['preset4', [[21.8, 3.0, 21.8], [1, 0.157, 0.192, 0.355, 0.200, 0.600, 3, 4, 4, 0.025, 0.490]]],
  ['preset5', [[21.8, 3.0, 21.8], [1, 0.157, 0.232, 0.599, 0.337, 0.699, 3, 4, 4, 0.025, 0.290]]]
];

function upperCaseFirst(s) {
  return s.charAt(0).toUpperCase() + s.substr(1);
}

function getUpdateGroupMessage(groupName) {
  var values = $('.' + groupName + ' .range > div').map(function () {
    return $(this).data('realValue')();
  });
  var msg = 'Set' + upperCaseFirst(groupName) + ':' +
      Array.prototype.join.call(values, ',');
  return msg;
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
  var menu = $('#preset-menu');
  for (var i = 0; i < presets.length; ++i) {
    var presetName = presets[i][0];
    menu.append('<li><a href="#"'+
        'data-value="' + i + '">' + presetName + '</a></li>');
  }
  menu.menu({
    select: function (e, ui) {
      var presetIndex = ui.item.children('a:first').data('value');
      onPresetChanged(presetIndex);
    }
  });
}

function onPresetChanged(index) {
  var preset = presets[index];
  $('.kernel .range > div').each(function (i) {
    $(this).data('setRealValue')(preset[1][0][i]);
  });
  $('.smoother .range > div').each(function (i) {
    $(this).data('setRealValue')(preset[1][1][i]);
  });
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

  slider.bind('slidechange', function (e, ui) {
    el.children('span').text($(this).data('textValue')());
    updateGroup(group);
  });
  slider.data('realValue', function () {
    return sliderWidget.value();
  });
  slider.data('textValue', function () {
    return values[sliderWidget.value()];
  });
  slider.data('setRealValue', function (value) {
    sliderWidget.value(value);
    slider.trigger('slidechange');
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

  slider.bind('slidechange', function (e, ui) {
    el.children('span').text($(this).data('textValue')());
    updateGroup(group);
  });
  slider.data('realValue', function () {
    return sliderWidget.value() / mult;
  });
  slider.data('textValue', function () {
    return (sliderWidget.value() / mult).toFixed(prec);
  });
  slider.data('setRealValue', function (value) {
    sliderWidget.value(value * mult);
    slider.trigger('slide');
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
  $('body').layout({
    slidable: false,
    center__onresize: function (name, el) {
      $('#nacl_module').attr('width', el.width()).attr('height', el.height());
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
    animate: false,
  });

  $('#clear').button().click(function (e) {
    $('#nacl_module').get(0).postMessage('Clear:0');
  });
  $('#splat').button().click(function (e) {
    $('#nacl_module').get(0).postMessage('Splat');
  });
  $('#presets').menu({
    select: function (e, ui) {
      alert(ui.item.children('a:first').text());
    }
  });

  var groups = ['kernel', 'smoother'];
  for (var i = 0; i < groups.length; ++i) {
    makeSlidersForGroup(groups[i]);
  }
}

function makeEmbed() {
  // Generate embed using current sizes and values.
  var center = $('.ui-layout-center');
  $('#listener').append(
      '<embed id="nacl_module" src="smoothlife.nmf"' +
      'width="' + center.width() + '" ' +
      'height="' + center.height() + '" type="application/x-nacl"' +
      'msg1="' + getUpdateGroupMessage('kernel') + '" ' +
      'msg2="' + getUpdateGroupMessage('smoother') + '" ' +
      'msg3="Clear:0" msg4="Splat"' +
      'msg5="SetRunOptions:continuous" msg6="SetDrawOptions:simulation">');

  // jQuery events don't work with embed elements.
  $('#listener').get(0).addEventListener('message', function (e) {
    $('#fps').text(e.data);
  }, true);
}

$(document).ready(function (){
  initPresets();
  setupUI();
  makeEmbed();
});
