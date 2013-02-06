/**
 * Copyright 2013 Ben Smith. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
