// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include "ppapi/cpp/input_event.h"
#include "ppapi/cpp/var.h"

#include "kernel.h"
#include "simulation.h"
#include "smoother.h"
#include "smoothlife_instance.h"
#include "smoothlife_thread.h"
#include "smoothlife_view.h"
#include "task.h"

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
  // TODO(binji): make this nicer.
  message_map_.insert(MessageMap::value_type(
        "SetKernel", &SmoothlifeInstance::MessageSetKernel));
  message_map_.insert(MessageMap::value_type(
        "SetSmoother", &SmoothlifeInstance::MessageSetSmoother));
  message_map_.insert(MessageMap::value_type(
        "Clear", &SmoothlifeInstance::MessageClear));
  message_map_.insert(MessageMap::value_type(
        "Splat", &SmoothlifeInstance::MessageSplat));
  //0 12.0 3.0 12.0 0.100 0.278 0.365 0.267 0.445 4 4 4 0.028 0.147
  //1 31.8 3.0 31.8 0.157 0.092 0.098 0.256 0.607 4 4 4 0.015 0.340
  //1 21.8 3.0 21.8 0.157 0.192 0.200 0.355 0.600 4 4 4 0.025 0.490
  //1 21.8 3.0 21.8 0.157 0.232 0.337 0.599 0.699 4 4 4 0.025 0.290
  //2 12.0 3.0 12.0 0.115 0.269 0.340 0.523 0.746 4 4 4 0.028 0.147
  //2 12.0 3.0 12.0 0.415 0.269 0.350 0.513 0.756 4 4 4 0.028 0.147
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
  task_queue_ = new LockedObject<TaskQueue>(new TaskQueue);

  ThreadContext context;
  context.config = config;
  context.buffer = locked_buffer_;
  context.queue = task_queue_;
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
  static bool left_down_ = false;

  switch (event.GetType()) {
    case PP_INPUTEVENT_TYPE_MOUSEUP:
    case PP_INPUTEVENT_TYPE_MOUSEDOWN:
    case PP_INPUTEVENT_TYPE_MOUSEMOVE: {
      pp::MouseInputEvent mouse_event(event);
      if (mouse_event.GetButton() == PP_INPUTEVENT_MOUSEBUTTON_LEFT)
        left_down_ = event.GetType() == PP_INPUTEVENT_TYPE_MOUSEDOWN;

      if (left_down_)
        EnqueueTask(MakeFunctionTask(&SmoothlifeThread::TaskDrawFilledCircle,
                                     mouse_event.GetPosition().x(),
                                     mouse_event.GetPosition().y(),
                                     10,
                                     1.0));
      return true;
    }
    case PP_INPUTEVENT_TYPE_KEYUP:
    case PP_INPUTEVENT_TYPE_KEYDOWN:
      return true;
    default:
    case PP_INPUTEVENT_TYPE_UNDEFINED:
    case PP_INPUTEVENT_TYPE_MOUSEENTER:
    case PP_INPUTEVENT_TYPE_MOUSELEAVE:
    case PP_INPUTEVENT_TYPE_WHEEL:
    case PP_INPUTEVENT_TYPE_RAWKEYDOWN:
    case PP_INPUTEVENT_TYPE_CHAR:
    case PP_INPUTEVENT_TYPE_CONTEXTMENU:
    case PP_INPUTEVENT_TYPE_IME_COMPOSITION_START:
    case PP_INPUTEVENT_TYPE_IME_COMPOSITION_UPDATE:
    case PP_INPUTEVENT_TYPE_IME_COMPOSITION_END:
    case PP_INPUTEVENT_TYPE_IME_TEXT:
    case PP_INPUTEVENT_TYPE_TOUCHSTART:
    case PP_INPUTEVENT_TYPE_TOUCHMOVE:
    case PP_INPUTEVENT_TYPE_TOUCHEND:
    case PP_INPUTEVENT_TYPE_TOUCHCANCEL:
      return false;
  }
}

void SmoothlifeInstance::HandleMessage(const pp::Var& var_message) {
  if (!var_message.is_string())
    return;
  std::string message = var_message.AsString();
  size_t colon = message.find(':');

  std::string function = message.substr(0, colon);
  std::vector<std::string> params;

  if (colon != std::string::npos) {
    size_t param_start = colon + 1;
    size_t comma;
    do {
      comma = message.find(',', param_start);
      params.push_back(message.substr(param_start, comma - param_start));
      param_start = comma + 1;
    } while (comma != std::string::npos);
  }

  MessageMap::iterator func_iter = message_map_.find(function);
  if (func_iter != message_map_.end())
    (this->*func_iter->second)(params);
}

void SmoothlifeInstance::MessageSetKernel(const ParamList& params) {
  if (params.size() != 3)
    return;

  KernelConfig config;
  config.ra = strtod(params[0].c_str(), NULL);
  config.rr = strtod(params[1].c_str(), NULL);
  config.rb = strtod(params[2].c_str(), NULL);
  EnqueueTask(MakeFunctionTask(&SmoothlifeThread::TaskSetKernel, config));
}

void SmoothlifeInstance::MessageSetSmoother(const ParamList& params) {
  if (params.size() != 11)
    return;

  SmootherConfig config;
  config.timestep.type = static_cast<Timestep>(atoi(params[0].c_str()));
  config.timestep.dt = strtod(params[1].c_str(), NULL);
  config.b1 = strtod(params[2].c_str(), NULL);
  config.d1 = strtod(params[3].c_str(), NULL);
  config.b2 = strtod(params[4].c_str(), NULL);
  config.d2 = strtod(params[5].c_str(), NULL);
  config.mode = static_cast<SigmoidMode>(atoi(params[6].c_str()));
  config.sigmoid = static_cast<Sigmoid>(atoi(params[7].c_str()));
  config.mix = static_cast<Sigmoid>(atoi(params[8].c_str()));
  config.sn = strtod(params[9].c_str(), NULL);
  config.sm = strtod(params[10].c_str(), NULL);
  EnqueueTask(MakeFunctionTask(&SmoothlifeThread::TaskSetSmoother, config));
}

void SmoothlifeInstance::MessageClear(const ParamList& params) {
  if (params.size() != 1)
    return;

  double color = strtod(params[0].c_str(), NULL);
  EnqueueTask(MakeFunctionTask(&SmoothlifeThread::TaskClear, color));
}

void SmoothlifeInstance::MessageSplat(const ParamList& params) {
  if (params.size() != 0)
    return;

  EnqueueTask(MakeFunctionTask(&SmoothlifeThread::TaskSplat));
}

void SmoothlifeInstance::EnqueueTask(Task* task) {
  TaskQueue* queue = task_queue_->Lock();
  queue->push_back(task);
  task_queue_->Unlock();
}
