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

#include "kernel.h"
#include "simulation.h"
#include "smoother.h"
#include "smoothlife_instance.h"
#include "smoothlife_thread.h"
#include "smoothlife_view.h"

SmoothlifeInstance::SmoothlifeInstance(PP_Instance instance)
    : pp::Instance(instance),
      factory_(this),
      view_(NULL),
      is_initial_view_change_(true),
      thread_(NULL),
      locked_buffer_(NULL),
      task_queue_(NULL) {
  // Request to receive input events.
  RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE | PP_INPUTEVENT_CLASS_KEYBOARD);
}

SmoothlifeInstance::~SmoothlifeInstance() {
  delete task_queue_;
  delete locked_buffer_;
  delete thread_;
  delete view_;
}

bool SmoothlifeInstance::Init(uint32_t argc, const char* argn[],
                              const char* argv[]) {
  SimulationConfig config;
  config.size = pp::Size(512, 512);
  config.kernel_config.ra = 12.0;
  config.kernel_config.rr = 3.0;
  config.kernel_config.rb = 12.0;
  config.smoother_config.timestep.type = TIMESTEP_SMOOTH2;
  config.smoother_config.timestep.dt = 0.115;
  config.smoother_config.b1 = 0.269;
  config.smoother_config.b2 = 0.340;
  config.smoother_config.d1 = 0.523;
  config.smoother_config.d2 = 0.746;
  config.smoother_config.mode = SIGMOID_MODE_4;
  config.smoother_config.sigmoid = SIGMOID_SMOOTH;
  config.smoother_config.mix = SIGMOID_SMOOTH;
  config.smoother_config.sn = 0.028;
  config.smoother_config.sm = 0.147;

  AlignedReals* buffer = new AlignedReals(config.size);
  locked_buffer_ = new LockedObject<AlignedReals>(buffer);

  ThreadContext context;
  context.config = config;
  context.buffer = locked_buffer_;
  thread_ = new SmoothlifeThread(context);
  view_ = new SmoothlifeView(locked_buffer_);

  return true;
}

void SmoothlifeInstance::DidChangeView(const pp::View& view) {
  if (!view_->DidChangeView(this, view, is_initial_view_change_)) {
    PostMessage(pp::Var(
        "ERROR DidChangeView failed. Could not bind graphics?"));
    return;
  }

  is_initial_view_change_ = false;
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
