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

  [[3.2,34,14.8],[1,0.26,0.172,0.370,0.740,0.697,1,1,4,0.772,0.280],[0,"#3b8191",18,"#66f24f",82,"#698ffe",100]],
  [[15.3,5.5,33.2],[1,0.746,0.283,0.586,0.702,0.148,1,2,0,0.379,0.633],[1,"#42ae80",77,"#fd1e2e",79,"#58103f",93,"#cf9750",96]],
  [[2.5,3.5,7.7],[3,0.666,0.779,0.002,0.558,0.786,3,1,3,0.207,0.047],[0,"#a2898d",78,"#60d14e",86,"#5c4dea",90]],
[[7.6,7.6,9.0],[1,0.158,0.387,0.234,0.810,0.100,3,0,2,0.029,0.533],[0,"#568b8a",5,"#18ce42",92]]
];

//  [[2.6,5.6,6.9],[1,0.899,0.666,0.374,0.675,0.890,1,0,0,0.200,0.692],[0,"#ebaab0",95,"#f028d3",97,"#56b67a",99,"#421e93",100]],
// [[13.4,20.1,8.6],[3,0.946,0.170,0.193,0.778,0.304,2,3,1,0.825,0.926],[1,"#52d2a1",3,"#c7c5ca",46,"#be6e88",72,"#f5a229",79,"#f0e0d1",94,"#6278d8",100]],
// [[22.51918929279782,29.940602727001533,5.733902857173234],[4,0.331774118822068,0.5793631710112095,0.1159830829128623,0.5955206523649395,0.4240599174518138,2,4,0,0.052642558235675097,0.741997601930052],[0,"#25f318",45,"#1a0e02",93]]

var colorPresets = [
  [0, '#ffffff', 0, '#000000', 100],
  [0, '#000000', 0, '#ffffff', 100],
  [0, '#000000', 0, '#ff00ff', 50, '#000000', 100],
  [0,"#000000",0,"#0f8a84",38,"#f5f5c1",43,"#158a34",70,"#89e681",100],
  [1,"#000000",3,"#f5f5c1",8,"#158a34",17,"#89e681",20],
  [0,"#36065e",0,"#c24242",77,"#8a19b0",91,"#ff9900",99,"#f5c816",99],
  [0,"#93afd9",11,"#9cf0ff",92,"#edfdff",100],
  [0,"#000000",11,"#ffffff",22,"#19a68a",85,"#6b0808",98],
  [0,"#0a1340",0,"#ffffff",55,"#4da8a3",83,"#2652ab",99,"#2f1e75",46],
  [0,"#b8cfcf",0,"#3f5a5c",77,"#1a330a",91,"#c0e0dc",99],
  [0,"#4d3e3e",0,"#9a1ac9",77,"#aaf09e",100],
  [1,"#52d2a1",3,"#c7c5ca",46,"#be6e88",72,"#f5a229",79,"#f0e0d1",94,"#6278d8",100]
];

function $(id) {
  return document.getElementById(id);
}

function moduleDidLoad() {
  addFunctions();
  setSize(256);
  setMaxScale(0);
  loadPreset(0);
}

function noEvent(func) {
  return function(event) {
    func();
  };
}

function attachListeners() {
  $('functionValue').addEventListener('change', onFunctionChanged, false);

  $('preset').addEventListener('change', onLoadPreset, false);
  $('resetPreset').addEventListener('click', onLoadPreset, false);
  $('colorPreset').addEventListener('change', onLoadColorPreset, false);
  $('resetColorPreset').addEventListener('click', onLoadColorPreset, false);
  $('randomize').addEventListener('click', randomize, false);
  $('zero').addEventListener('click', function() { clear(0); }, false);
  $('splat').addEventListener('click', splat, false);
  $('listener').addEventListener('message', handleMessage, true);

  // $('setPaletteNumColorstops').addEventListener('change', onNumColorstopsChanged, false);
}

function handleMessage(e) {
  document.getElementById('fps').textContent = 'FPS: ' + e.data.toFixed(2);
}

// From MDN:
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/random

function getRandomFloat(min, max) {
    return Math.random() * (max - min) + min;
}

function getRandomInt(min, max) {
  return Math.floor(Math.random() * (max - min + 1) + min);
}

function getRandomColorComponent() {
  var comp = getRandomInt(0, 255);
  return ('00' + comp.toString(16)).slice(-2);
}

function getRandomColor() {
  var r = getRandomColorComponent();
  var g = getRandomColorComponent();
  var b = getRandomColorComponent();
  return '#' + r + g + b;
}

function randomize() {
  var kernel = [0, 0, 0];
  kernel[0] = getRandomFloat(0, 35);
  kernel[1] = getRandomFloat(0, 35);
  kernel[2] = getRandomFloat(0, 10);

  var smoother = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
  smoother[0] = getRandomInt(0, 4);  // type
  smoother[1] = getRandomFloat(0, 1);  // dt
  smoother[2] = getRandomFloat(0, 1);  // b1
  smoother[3] = getRandomFloat(0, 1);  // d1
  smoother[4] = getRandomFloat(0, 1);  // b2
  smoother[5] = getRandomFloat(0, 1);  // d2
  smoother[6] = getRandomInt(0, 3);  // mode
  smoother[7] = getRandomInt(0, 4);  // sigmoid
  smoother[8] = getRandomInt(0, 4);  // mix
  smoother[9] = getRandomFloat(0, 1);  // sn
  smoother[10] = getRandomFloat(0, 1);  // sm

  var palette = [];
  var stops = getRandomInt(2, 8);
  palette.push(getRandomInt(0, 1));
  var lastStop = 0;
  for (var i = 0; i < stops; ++i) {
    var stop = getRandomInt(lastStop, 100);
    lastStop = stop;

    palette.push(getRandomColor());
    palette.push(stop);
  }

  // Log it in easily copyable format:
  console.log('Generated: ' + JSON.stringify([kernel, smoother, palette]));

  clear(0);
  setKernel.apply(null, kernel);
  setSmoother.apply(null, smoother);
  setPalette.apply(null, palette);
  splat();
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
  splat();
}

function onLoadColorPreset(e) {
  var index = parseInt(document.getElementById('colorPreset').value, 10);
  loadColorPreset(index);
}

function loadColorPreset(index) {
  var preset = colorPresets[index];
  setPalette.apply(null, preset);
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

var functions = [
  {name: 'clear', params: [
      {name: 'color', type: 'range', min: 0, max: 1, step: 0.1}]},
  {name: 'setSize', params: [
      {name: 'size', type: 'select', values: [
          {name: '256x256', value: 256},
          {name: '384x384', value: 384},
          {name: '512x512', value: 512}]}]},
  {name: 'setMaxScale', params: [
      {name: 'scale', type: 'range', min: 0, max: 5, step: 0.1}]},
  {name: 'setThreadCount', params: [
      {name: 'threadCount', type: 'select', values: [
          {name: '1 Thread', value: 1},
          {name: '2 Threads', value: 2},
          {name: '4 Threads', value: 4},
          {name: '6 Threads', value: 6},
          {name: '8 Threads', value: 8},
          {name: '12 Threads', value: 12}]}]},
  {name: 'setBrush', params: [
      {name: 'radius', type: 'range', min: 0, max: 50, step: 0.1},
      {name: 'color', type: 'range', min: 0, max: 1, step: 0.1}]},
  {name: 'setKernel', params: [
      {name: 'discRadius', type: 'range', min: 0, max: 50, step: 0.1},
      {name: 'ringRadius', type: 'range', min: 0, max: 50, step: 0.1},
      {name: 'blendRadius', type: 'range', min: 0, max: 50, step: 0.1}]},
  {name: 'setSmoother', params: [
      {name: 'type', type: 'range', min: 0, max: 4, step: 1},
      {name: 'dt', type: 'range', min: 0, max: 1, step: 0.1},
      {name: 'b1', type: 'range', min: 0, max: 1, step: 0.001},
      {name: 'd1', type: 'range', min: 0, max: 1, step: 0.001},
      {name: 'b2', type: 'range', min: 0, max: 1, step: 0.001},
      {name: 'd2', type: 'range', min: 0, max: 1, step: 0.001},
      {name: 'mode', type: 'range', min: 0, max: 4, step: 1},
      {name: 'sigmoid', type: 'range', min: 0, max: 3, step: 1},
      {name: 'mix', type: 'range', min: 0, max: 3, step: 1},
      {name: 'sn', type: 'range', min: 0, max: 1, step: 0.1},
      {name: 'sm', type: 'range', min: 0, max: 1, step: 0.1}]},
];

var values = {};

function addFunctions() {
  var fnsEl = $('functions');
  for (var i = 0; i < functions.length; ++i) {
    var f = functions[i];
    var fnEl = document.createElement('table');
    var fnVals = {};
    var fnUpdateViews = {};

    fnEl.classList.add('function');
    fnEl.setAttribute('id', f.name);
    fnEl.setAttribute('hidden', 'true');
    for (var j = 0; j < f.params.length; ++j) {
      var p = f.params[j];
      var paramEl = document.createElement('tr');
      var nameEl = document.createElement('td');
      paramEl.classList.add('param');
      nameEl.textContent = p.name;
      paramEl.appendChild(nameEl);

      var valueEl = document.createElement('td');
      paramEl.appendChild(valueEl);
      if (p.type === 'range') {
        var rangeEl = document.createElement('input');
        rangeEl.setAttribute('type', 'range');
        rangeEl.setAttribute('min', p.min);
        rangeEl.setAttribute('max', p.max);
        rangeEl.setAttribute('step', p.step);
        rangeEl.setAttribute('value', p.min);
        valueEl.appendChild(rangeEl);

        var textEl = document.createElement('span');
        textEl.classList.add('value');
        textEl.textContent = p.min;
        valueEl.appendChild(textEl);

        // Listener to bind view => model.
        rangeEl.addEventListener('change', function(fnVals, f, p, textEl) {
          return function() {
            // Cheesy, but OK for now. detect float vs. int by step size.
            var isInt = p.step === 1;
            var value;
            if (isInt)
              value = parseInt(this.value, 10);
            else
              value = parseFloat(this.value);
            fnVals[p.name] = value;
            textEl.textContent = this.value;
            // post these new values to the module.
            window[f.name]();
          };
        }(fnVals, f, p, textEl), false);

        // Function to bind model => view.
        fnUpdateViews[p.name] = function(fnVals, p, rangeEl, textEl) {
          return function() {
            rangeEl.value = fnVals[p.name];
            textEl.textContent = fnVals[p.name];
          };
        }(fnVals, p, rangeEl, textEl);

        // Set initial model value.
        fnVals[p.name] = p.min;
      } else if (p.type === 'select') {
        var selectEl = document.createElement('select');
        for (var k = 0; k < p.values.length; ++k) {
          var v = p.values[k];
          var optionEl = document.createElement('option');
          optionEl.textContent = v.name;
          optionEl.setAttribute('value', v.value);
          selectEl.appendChild(optionEl);
        }
        valueEl.appendChild(selectEl);

        // Listener to bind view => model.
        selectEl.addEventListener('change', function(fnVals, f, p) {
          return function() {
            fnVals[p.name] = parseInt(this.value, 10);
            // post these new values to the module.
            window[f.name]();
          }
        }(fnVals, f, p), false);

        // Function to bind model => view.
        fnUpdateViews[p.name] = function(fnVals, p, selectEl) {
          return function() {
            selectEl.value = fnVals[p.name];
          }
        }(fnVals, p, selectEl);

        // Set initial model value.
        fnVals[p.name] = p.values[0].value;
      }
      fnEl.appendChild(paramEl);
    }
    fnsEl.appendChild(fnEl);

    // Set all initial values for this function.
    values[f.name] = fnVals;

    // Function to update view of all functions parameters, given the values
    // in the model.
    // For a function named "foo", this function is called "fooUpdateView".
    window[f.name + 'UpdateView'] = function(fnUpdateViews) {
      return function() {
        for (var pName in fnUpdateViews) {
          fnUpdateViews[pName]();
        }
      }
    }(fnUpdateViews);

    // Add function to post current values to NaCl module.
    window[f.name] = function(f) {
      return function() {
        var fnVals = values[f.name];
        if (arguments.length) {
          // This is being called externally, use the values passed in and
          // update the view.
          for (var i = 0; i < f.params.length; ++i) {
            var pName = f.params[i].name;
            fnVals[pName] = arguments[i];
          }
          window[f.name + 'UpdateView']();
        }

        // Make the command dict and send it to the module.
        var cmd = {cmd: f.name};
        for (var pName in fnVals) {
          cmd[pName] = fnVals[pName];
        }

        postMessage(cmd);
      };
    }(f);
  }
}

function splat() {
  postMessage({cmd: 'splat'});
}

function getValueArg(arg, id) {
  if (arg !== undefined)
    return arg;
  else
    return document.getElementById(id).value;
}

function getIntArg(arg, id) {
  return parseInt(getValueArg(arg, id), 10);
}

function getFloatArg(arg, id) {
  return parseFloat(getValueArg(arg, id));
}

function getBoolArg(arg, id, trueVal, falseVal) {
  return getValueArg(arg, id) ? trueVal : falseVal;
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

function postMessage(msg) {
  //console.log(msg);
  common.naclModule.postMessage(msg);
}
