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

function updateGroups(groupName) {
  var values = $('.knobs.' + groupName + ' .range > div').map(function () {
    return $(this).data('realValue');
  });
  var msg = 'Set' + upperCaseFirst(groupName) + ':' +
      Array.prototype.join.call(values, ',');
  $('#nacl_module').get(0).postMessage(msg);
  updateTimeoutID[groupName] = null;
}

function updateGroup(groupName) {
  if (!updateTimeoutID[groupName]) {
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
  menu.menu({select: function (e, ui) {
    alert(ui.item.children('a:first').data('value'));
  }});
}

function onPresetChanged(e) {
  var presetIndex = document.getElementById('presets').selectedIndex;
  var preset = presets[presetIndex];

  var kernelPreset = preset[1][0];
  var kernelEls = document.querySelectorAll('tr.kernel input');
  for (var i = 0; i < kernelEls.length; ++i) {
    kernelEls[i].value = kernelPreset[i];
    sendChangeEvent(kernelEls[i]);
  }

  var smootherPreset = preset[1][1];
  var smootherEls = document.querySelectorAll('tr.smooth input');
  for (var i = 0; i < smootherEls.length; ++i) {
    smootherEls[i].value = smootherPreset[i];
    sendChangeEvent(smootherEls[i]);
  }
}

$(document).ready(function (){
  //window.setInterval(updateGroups, 200);
  initPresets();
  //onPresetChanged(null);

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

  // Generate embed using current sizes.
  var center = $('.ui-layout-center');
  $('#listener').append('<embed id="nacl_module" src="smoothlife.nmf"' +
      'width="' + center.width() + '" ' +
      'height="' + center.height() + '" type="application/x-nacl">');

  // jQuery events don't work with embed elements.
  $('#listener').get(0).addEventListener('message', function (e) {
    $('#fps').text(e.data);
  }, true);

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
  $('.knobs > div').each(function () {
    var el = $(this);
    var name = el.text();
    var prec = el.data('prec');
    var mult = Math.pow(10, prec);
    var values = el.data('values');
    if (values) {
      values = values.split(' ');
    }

    for (var i = 0; i < groups.length; ++i) {
      if (el.parent().hasClass(groups[i])) {
        var group = groups[i];
        break;
      }
    }

    el.empty().addClass('range');
    el.append('<label>' + name + '</label>');
    if (prec) {
      el.append('<span>' + el.data('value') + '</span>');
      var slider = $('<div/>').slider({
        value: el.data('value') * mult,
        min: el.data('min') * mult,
        max: el.data('max') * mult,
        range: 'min',
        slide: function (e, ui) {
          var realValue = ui.value / mult;
          $(this).data('realValue', realValue);
          el.children('span').text(realValue);
          updateGroup(group);
        }
      });
      slider.data('realValue', el.data('value'));
      el.append(slider);
    } else {
      el.append('<span>' + values[el.data('value')] + '</span>');
      var slider = $('<div/>').slider({
        value: el.data('value'),
        min: 0,
        max: values.length - 1,
        range: 'min',
        slide: function (e, ui) {
          var realValue = ui.value;
          $(this).data('realValue', realValue);
          el.children('span').text(values[ui.value]);
          updateGroup(group);
        },
      });
      slider.data('realValue', el.data('value'));
      el.append(slider);
    }
  });
});
