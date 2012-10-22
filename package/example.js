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

function moduleDidLoad() {
  window.setInterval(updateGroups, 200);
  initPresetSelect();
  onPresetChanged(null);
}

function handleMessage() {
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

function attachListeners() {
  var rangeEls = document.querySelectorAll('input[type=range]');
  for (var i = 0; i < rangeEls.length; ++i) {
    var rangeEl = rangeEls[i];
    var valueEl = rangeEl.parentElement.nextElementSibling.children[0];
    var id = rangeEl.parentElement.parentElement.id;
    var group = rangeEl.parentElement.parentElement.className;
    rangeEl.addEventListener('change', updateValue(valueEl, id, group));
  }

  document.getElementById('clear').addEventListener('click', onClearClicked);
  document.getElementById('splat').addEventListener('click', onSplatClicked);
  document.getElementById('presets').addEventListener('change', onPresetChanged);
}

function onClearClicked(e) {
  common.naclModule.postMessage('Clear:0');
}

function onSplatClicked(e) {
  common.naclModule.postMessage('Splat');
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
