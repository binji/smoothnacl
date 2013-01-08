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

  $('.tabs').tabs({
    active: 1,
    heightStyle: 'fill',
  });
});
