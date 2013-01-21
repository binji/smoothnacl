/**
 * Copyright (c) 2012 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 **/
"use strict"

$(document).ready(function () {
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
      $('#presetMenu').masonry('reload');
    },
    livePaneResizing: true
  });

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
});
