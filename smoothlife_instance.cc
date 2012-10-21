// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <string>
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/input_event.h"
#include "ppapi/cpp/rect.h"
#include "ppapi/cpp/var.h"

#include "smoothlife_instance.h"
#include "smoothlife_view.h"

namespace {

const int kUpdateInterval = 17;

}  // namespace

SmoothlifeInstance::SmoothlifeInstance(PP_Instance instance)
    : pp::Instance(instance),
      factory_(this),
      view_(NULL),
      is_initial_view_change_(true) {
  // Request to receive input events.
  RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE | PP_INPUTEVENT_CLASS_KEYBOARD);
}

SmoothlifeInstance::~SmoothlifeInstance() {
  delete view_;
}

bool SmoothlifeInstance::Init(uint32_t argc, const char* argn[], const char* argv[]) {
  view_ = new SmoothlifeView();
  return true;
}

void SmoothlifeInstance::DidChangeView(const pp::View& view) {
  if (!view_->DidChangeView(this, view, is_initial_view_change_)) {
    PostMessage(pp::Var(
        "ERROR DidChangeView failed. Could not bind graphics?"));
    return;
  }

  if (is_initial_view_change_) {
    ScheduleUpdate();
    is_initial_view_change_ = false;
  }
}

bool SmoothlifeInstance::HandleInputEvent(const pp::InputEvent& event) {
  if (event.GetType() == PP_INPUTEVENT_TYPE_MOUSEUP ||
      event.GetType() == PP_INPUTEVENT_TYPE_MOUSEDOWN) {
    // By notifying the browser mouse clicks are handled, the application window
    // is able to get focus and receive key events.
    return true;
  } else if (event.GetType() == PP_INPUTEVENT_TYPE_KEYUP) {
    return true;
  } else if (event.GetType() == PP_INPUTEVENT_TYPE_KEYDOWN) {
    return true;
  }
  return false;
}

void SmoothlifeInstance::HandleMessage(const pp::Var& var_message) {
  if (!var_message.is_string())
    return;
}

void SmoothlifeInstance::ScheduleUpdate() {
  pp::Module::Get()->core()->CallOnMainThread(
      kUpdateInterval,
      factory_.NewCallback(&SmoothlifeInstance::UpdateCallback));
}

void SmoothlifeInstance::UpdateCallback(int32_t result) {
  // This is the game loop; UpdateCallback schedules another call to itself to
  // occur kUpdateInterval milliseconds later.
  ScheduleUpdate();
}
