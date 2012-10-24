var kernelValues;
var kernelDirty = false;
var smootherValues;
var smootherDirty = false;
var custom = 1;
var presets = [
  ['preset1', [[12.0, 3.0, 12.0], [2, 0.115, 0.269, 0.523, 0.340, 0.746, 3, 4, 4, 0.028, 0.147]]],
  ['preset2', [[12.0, 3.0, 12.0], [0, 0.100, 0.278, 0.267, 0.365, 0.445, 3, 4, 4, 0.028, 0.147]]],
  ['preset3', [[31.8, 3.0, 31.8], [1, 0.157, 0.092, 0.256, 0.098, 0.607, 3, 4, 4, 0.015, 0.340]]],
  ['preset4', [[21.8, 3.0, 21.8], [1, 0.157, 0.192, 0.355, 0.200, 0.600, 3, 4, 4, 0.025, 0.490]]],
  ['preset5', [[21.8, 3.0, 21.8], [1, 0.157, 0.232, 0.599, 0.337, 0.699, 3, 4, 4, 0.025, 0.290]]]
];

function handleMessage(msg) {
}

function getValues(x) {
  return x.value;
}

function updateGroups() {
  if (kernelDirty) {
    common.naclModule.postMessage('SetKernel:' + kernelValues.join(','));
    kernelDirty = false;
  }
  if (smootherDirty) {
    common.naclModule.postMessage('SetSmoother:' + smootherValues.join(','));
    smootherDirty = false;
  }
}

function updateGroup(groupName) {
  if (groupName == 'kernel') {
    var kernelEls = document.querySelectorAll('tr.kernel input');
    kernelValues = Array.prototype.map.call(kernelEls, getValues);
    kernelDirty = true;
  } else if (groupName == 'smooth') {
    var smoothEls = document.querySelectorAll('tr.smooth input');
    smootherValues = Array.prototype.map.call(smoothEls, getValues);
    smootherDirty = true;
  }
}

function updateValue(valueEl, id, group) {
  var lookup = null;

  if (id == 'timestep') {
    lookup = ['discrete', 'smooth1', 'smooth2', 'smooth3'];
  } else if (id == 'mode') {
    lookup = ['mode1', 'mode2', 'mode3', 'mode4'];
  } else if (id == 'sigmoid' || id == 'mix') {
    lookup = ['hard', 'linear', 'hermite', 'sin', 'smooth'];
  }

  return function(e) {
    if (lookup)
      valueEl.textContent = lookup[e.target.value];
    else
      valueEl.textContent = e.target.value;

    updateGroup(group);
  }
}

function sendChangeEvent(el) {
  var evt = document.createEvent('Event');
  evt.initEvent('change', true, true);
  el.dispatchEvent(evt);
}

function initPresetSelect() {
  var selectEl = document.getElementById('presets');
  for (var i = 0; i < presets.length; ++i) {
    var optionEl = document.createElement('option');
    var presetName = presets[i][0];
    optionEl.setAttribute('value', i);
    var textNode = document.createTextNode(presetName);
    optionEl.appendChild(textNode);
    selectEl.appendChild(optionEl);
  }
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
  //initPresetSelect();
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
  $('.knobs div').each(function () {
    var el = $(this);
    var name = el.text();
    var prec = el.data('prec');
    var mult = Math.pow(10, prec);
    var values = el.data('values');
    if (values) {
      values = values.split(' ');
    }

    el.empty().addClass('range');
    el.append('<label>' + name + '</label>');
    if (prec) {
      el.append('<span>' + el.data('value') + '</span>');
      el.append($('<div/>').slider({
        value: el.data('value') * mult,
        min: el.data('min') * mult,
        max: el.data('max') * mult,
        range: 'min',
        slide: function (e, ui) {
          el.children('span').text((ui.value / mult).toFixed(prec));
        }
      }));
    } else {
      el.append('<span>' + values[el.data('value')] + '</span>');
      el.append($('<div/>').slider({
        value: el.data('value'),
        min: 0,
        max: values.length - 1,
        range: 'min',
        slide: function (e, ui) {
          el.children('span').text(values[ui.value]);
        }
      }));
    }
  });
});
