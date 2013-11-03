var presets = [
  [[15.2,32.1,7.6],[3,0.329,0.15,0.321,0.145,0.709,3,2,4,0.269,0.662],[0,"#000000",3,"#f5f5c1",12,"#158a34",68,"#89e681",100]],
  [[15.2,32.1,5],[3,0.273,0.117,0.288,0.243,0.348,3,2,4,0.269,0.662],[1,"#000000",3,"#f5f5c1",8,"#158a34",17,"#89e681",20]],
  [[4,12,1],[2,0.115,0.269,0.523,0.34,0.746,3,4,4,0.028,0.147],[0,"#36065e",0,"#c24242",77,"#8a19b0",91,"#ff9900",99,"#f5c816",99]],
  [[4,12,1],[3,0.12,0.218,0.267,0.365,0.445,3,4,4,0.028,0.147],[0,"#000000",0,"#0f8a84",38,"#f5f5c1",43,"#158a34",70,"#89e681",100]],
  [[4,12,1],[0,0.09,0.276,0.27,0.365,0.445,1,4,4,0.028,0.147],[0,"#93afd9",11,"#9cf0ff",92,"#edfdff",100]],
  [[10.4,12,1],[2,0.082,0.302,0.481,0.35,0.749,2,3,4,0.028,0.147],[0,"#000000",11,"#ffffff",22,"#19a68a",85,"#6b0808",98]],
  [[7.8,27.2,2.6],[3,0.21,0.714,0.056,0.175,0.838,2,0,2,0.132,0.311],[0,"#0a1340",0,"#ffffff",55,"#4da8a3",83,"#2652ab",99,"#2f1e75",46]],
  [[4,12,1],[2,0.115,0.269,0.496,0.34,0.767,3,4,4,0.028,0.147],[0,"#b8cfcf",0,"#3f5a5c",77,"#1a330a",91,"#c0e0dc",99]],
  [[10.6,31.8,1],[1,0.157,0.092,0.256,0.098,0.607,3,4,4,0.015,0.34],[0,"#4d3e3e",0,"#9a1ac9",77,"#aaf09e",100]],
];

function $(id) {
  return document.getElementById(id);
}

function attachListeners() {
  $('functionValue').addEventListener('change', onFunctionChanged, false);
  $('setThreadCountThreadCount').addEventListener('change', onThreadCountChanged, false);
  $('setPaletteNumColorstops').addEventListener('change', onNumColorstopsChanged, false);
  $('execute').addEventListener('click', onExecute, false);
  $('loadPreset').addEventListener('click', onLoadPreset, false);
  $('listener').addEventListener('message', handleMessage, true);
}

function handleMessage(e) {
  document.getElementById('fps').textContent = 'FPS: ' + e.data.toFixed(2);
}

function onLoadPreset(e) {
  var index = parseInt(document.getElementById('preset').value, 10);
  loadPreset(index);
}

function loadPreset(index) {
  var preset = presets[index];
  clear(0);
  setKernel.apply(null, preset[0]);
  setSmoother.apply(null, preset[1]);
  setPalette.apply(null, preset[2]);
  //setDrawOptions('simulation');
  //setRunOptions('simulation', 'run');
  splat();
}

function onFunctionChanged(e) {
  var divId = this.value;
  var functionEls = document.querySelectorAll('.function');
  for (var i = 0; i < functionEls.length; ++i) {
    var visible = functionEls[i].id === divId;
    if (functionEls[i].id === divId)
      functionEls[i].removeAttribute('hidden');
    else
      functionEls[i].setAttribute('hidden');
  }
}

function onThreadCountChanged(e) {
  var threadCount = parseInt(this.value, 10);
  $('setThreadCountValue').textContent = threadCount;
}

function onNumColorstopsChanged(e) {
  var colorstops = parseInt(this.value, 10);
  var colorstopEl = document.getElementById('colorstops');

  // remove all child elements.
  while (colorstopEl.firstChild) {
    colorstopEl.removeChild(colorstopEl.firstChild);
  }

  if (colorstops >= 0 && colorstops < 8) {
    for (var i = 0; i < colorstops; ++i) {
      var myEl = document.createElement('div');
      myEl.appendChild(document.createTextNode('Color'+i+':'));
      var inputEl = document.createElement('input');
      inputEl.setAttribute('type', 'text');
      myEl.appendChild(inputEl);
      colorstopEl.appendChild(myEl);

      myEl = document.createElement('div');
      myEl.appendChild(document.createTextNode('Stop'+i+':'));
      inputEl = document.createElement('input');
      inputEl.setAttribute('type', 'text');
      myEl.appendChild(inputEl);
      colorstopEl.appendChild(myEl);
    }
  }
}

function onExecute(e) {
  var fun = document.getElementById('functionValue').value;
  window[fun].apply(null);
}

function getIntArg(arg, id) {
  return parseInt(arg || document.getElementById(id).value, 10);
}

function getFloatArg(arg, id) {
  return parseFloat(arg || document.getElementById(id).value);
}

function getValueArg(arg, id) {
  return arg || document.getElementById(id).value;
}

function getBoolArg(arg, id, trueVal, falseVal) {
  return (arg || document.getElementById(id).checked) ? trueVal : falseVal;
}

function clear() {
  var color = getFloatArg(arguments[0], 'clearColor');
  postMessage({cmd: 'clear', color: color});
}

function setSize() {
  var size = getIntArg(arguments[0], 'setSizeSize');
  postMessage({cmd: 'setSize', size: size});
}

function setThreadCount() {
  var threadCount = getIntArg(arguments[0], 'setThreadCountThreadCount');
  postMessage({cmd: 'setThreadCount', threadCount: threadCount});
}

function setBrush() {
  var radius = getFloatArg(arguments[0], 'setBrushRadius');
  var color = getFloatArg(arguments[1], 'setBrushColor');
  postMessage({cmd: 'setBrush', radius: radius, color: color});
}

function setKernel() {
  var discRadius = getFloatArg(arguments[0], 'setKernelDiscRadius');
  var ringRadius = getFloatArg(arguments[1], 'setKernelRingRadius');
  var blendRadius = getFloatArg(arguments[2], 'setKernelBlendRadius');
  postMessage({
    cmd: 'setKernel',
    discRadius: discRadius,
    ringRadius: ringRadius,
    blendRadius: blendRadius});
}

function setPalette() {
  var repeating = getBoolArg(arguments[0], 'setPaletteRepeating',
                             true, false);
  var colors = []
  var stops = []
  if (arguments.length > 1) {
    for (var i = 1; i < arguments.length; i += 2) {
      colors.push(arguments[i]);
      stops.push(arguments[i + 1]);
    }
  } else {
    var inputEls = document.querySelectorAll('#colorstops input');
    for (var i = 0; i < inputEls.length; i += 2) {
      colors.push(inputEls[i].value);
      stops.push(parseFloat(inputEls[i + 1].value));
    }
  }

  postMessage({
    cmd: 'setPalette',
    repeating: repeating,
    colors: colors,
    stops: stops});
}

function setSmoother() {
  var type = getIntArg(arguments[0], 'setSmootherType');
  var dt = getFloatArg(arguments[1], 'setSmootherDt');
  var b1 = getFloatArg(arguments[2], 'setSmootherB1');
  var d1 = getFloatArg(arguments[3], 'setSmootherD1');
  var b2 = getFloatArg(arguments[4], 'setSmootherB2');
  var d2 = getFloatArg(arguments[5], 'setSmootherD2');
  var mode = getIntArg(arguments[6], 'setSmootherMode');
  var sigmoid = getIntArg(arguments[7], 'setSmootherSigmoid');
  var mix = getIntArg(arguments[8], 'setSmootherMix');
  var sn = getFloatArg(arguments[9], 'setSmootherSn');
  var sm = getFloatArg(arguments[10], 'setSmootherSm');
  postMessage({
    cmd: 'setSmoother',
    type: type,
    dt: dt,
    b1: b1,
    d1: d1,
    b2: b2,
    d2: d2,
    mode: mode,
    sigmoid: sigmoid,
    mix: mix,
    sn: sn,
    sm: sm});
}

function splat() {
  postMessage({cmd: 'splat'});
}

function postMessage(msg) {
  console.log(msg);
  common.naclModule.postMessage(msg);
}
