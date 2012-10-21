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
#include "kernel.h"
#include "smoother.h"
#include "simulation.h"

namespace {

const pp::Size kDefaultSimulationSize(512, 512);

}  // namespace

SmoothlifeInstance::SmoothlifeInstance(PP_Instance instance)
    : pp::Instance(instance),
      factory_(this),
      view_(NULL),
      is_initial_view_change_(true),
      thread_create_result_(0),
      quit_(false),
      locked_buffer_(new AlignedReals(kDefaultSimulationSize)),
      sim_size_(kDefaultSimulationSize) {
  // Request to receive input events.
  RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE | PP_INPUTEVENT_CLASS_KEYBOARD);
}

SmoothlifeInstance::~SmoothlifeInstance() {
  quit_ = true;
  if (thread_create_result_ == 0)
    pthread_join(thread_, NULL);
  delete view_;
}

bool SmoothlifeInstance::Init(uint32_t argc, const char* argn[],
                              const char* argv[]) {
  kernel_config_.ra = 12.0;
  kernel_config_.rr = 3.0;
  kernel_config_.rb = 12.0;
  smoother_config_.timestep.type = TIMESTEP_SMOOTH2;
  smoother_config_.timestep.dt = 0.115;
  smoother_config_.b1 = 0.269;
  smoother_config_.b2 = 0.340;
  smoother_config_.d1 = 0.523;
  smoother_config_.d2 = 0.746;
  smoother_config_.mode = SIGMOID_MODE_4;
  smoother_config_.sigmoid = SIGMOID_SMOOTH;
  smoother_config_.mix = SIGMOID_SMOOTH;
  smoother_config_.sn = 0.028;
  smoother_config_.sm = 0.147;
  view_ = new SmoothlifeView(&locked_buffer_);

  thread_create_result_ = pthread_create(
      &thread_, NULL, &SmoothlifeThreadThunk, this);

  return true;
}

void SmoothlifeInstance::DidChangeView(const pp::View& view) {
  if (!view_->DidChangeView(this, view, is_initial_view_change_)) {
    PostMessage(pp::Var(
        "ERROR DidChangeView failed. Could not bind graphics?"));
    return;
  }

  if (is_initial_view_change_) {
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

void* SmoothlifeInstance::SmoothlifeThreadThunk(void* param) {
  SmoothlifeInstance* self = static_cast<SmoothlifeInstance*>(param);
  self->SmoothlifeThread();
  return NULL;
}

void SmoothlifeInstance::SmoothlifeThread() {
  //0 12.0 3.0 12.0 0.100 0.278 0.365 0.267 0.445 4 4 4 0.028 0.147
  //1 31.8 3.0 31.8 0.157 0.092 0.098 0.256 0.607 4 4 4 0.015 0.340
  //1 21.8 3.0 21.8 0.157 0.192 0.200 0.355 0.600 4 4 4 0.025 0.490
  //1 21.8 3.0 21.8 0.157 0.232 0.337 0.599 0.699 4 4 4 0.025 0.290
  //2 12.0 3.0 12.0 0.115 0.269 0.340 0.523 0.746 4 4 4 0.028 0.147
  //2 12.0 3.0 12.0 0.415 0.269 0.350 0.513 0.756 4 4 4 0.028 0.147
  Simulation simulation(sim_size_, kernel_config_, smoother_config_);
  simulation.Clear(0);
  simulation.inita2D(kernel_config_.ra);

  while (!quit_) {
    AlignedReals* out_data = locked_buffer_.Lock();
    std::copy(simulation.buffer().begin(), simulation.buffer().end(),
              out_data->begin());
    locked_buffer_.Unlock();

    simulation.Step();
  }
}
