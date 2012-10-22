var kernelValues;
var kernelDirty = false;
var smootherValues;
var smootherDirty = false;

function moduleDidLoad() {
  window.setInterval(updateGroups, 200);
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

function attachListeners() {
  var rangeEls = document.querySelectorAll('input[type=range]');
  for (var i = 0; i < rangeEls.length; ++i) {
    var rangeEl = rangeEls[i];
    var valueEl = rangeEl.parentElement.nextElementSibling.children[0];
    var id = rangeEl.parentElement.parentElement.id;
    var group = rangeEl.parentElement.parentElement.className;
    rangeEl.addEventListener('change', updateValue(valueEl, id, group));

    // Initialize the value.
    var evt = document.createEvent('Event');
    evt.initEvent('change', true, true);
    rangeEl.dispatchEvent(evt);
  }

}
