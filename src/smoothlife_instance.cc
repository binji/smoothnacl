// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <ppapi/c/pp_time.h>
#include <ppapi/cpp/core.h>
#include <ppapi/cpp/module.h>
#include "ppapi/cpp/input_event.h"
#include "ppapi/cpp/var.h"

#include "kernel.h"
#include "simulation.h"
#include "smoother.h"
#include "smoothlife_instance.h"
#include "smoothlife_thread.h"
#include "smoothlife_view.h"
#include "task.h"

namespace {

const int kUpdateInterval = 1000;

double GetTimeTicks() {
  return pp::Module::Get()->core()->GetTimeTicks();
}

}  // namespace


SmoothlifeInstance::SmoothlifeInstance(PP_Instance instance)
    : pp::Instance(instance),
      factory_(this),
      view_(NULL),
      thread_(NULL),
      sim_size_(512, 512),
      locked_buffer_(NULL),
      task_queue_(NULL),
      frames_drawn_(NULL),
      fullscreen_(this),
      is_initial_view_change_(true) {
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
  InitMessageMap();
  SimulationConfig config;
  config.size = sim_size_;

  AlignedReals* buffer = new AlignedReals(config.size);
  locked_buffer_ = new LockedObject<AlignedReals>(buffer);
  task_queue_ = new LockedObject<TaskQueue>(new TaskQueue);
  frames_drawn_ = new LockedObject<int>(new int(0));
  step_cond_ = new CondVar;

  ThreadContext context;
  context.config = config;
  context.run_options = kRunOptions_Continuous;
  context.buffer = locked_buffer_;
  context.queue = task_queue_;
  context.frames_drawn = frames_drawn_;
  context.step_cond = step_cond_;

  ParseInitMessages(argc, argn, argv, &context);

  thread_ = new SmoothlifeThread(context);
  view_ = new SmoothlifeView(locked_buffer_);

  return true;
}

void SmoothlifeInstance::ParseInitMessages(
    uint32_t argc,
    const char* argn[], const char* argv[],
    ThreadContext* context) {
  for (uint32_t i = 0; i < argc; ++i) {
    if (strncmp(argn[i], "msg", 3) == 0) {
      printf("Got message: %s\n", argv[i]);
      HandleMessage(pp::Var(argv[i]));
    } else if (strcmp(argn[i], "step") == 0) {
      context->run_options = kRunOptions_Step;
    }
  }
}

void SmoothlifeInstance::InitMessageMap() {
  message_map_.insert(MessageMap::value_type(
        "SetKernel", &SmoothlifeInstance::MessageSetKernel));
  message_map_.insert(MessageMap::value_type(
        "SetSmoother", &SmoothlifeInstance::MessageSetSmoother));
  message_map_.insert(MessageMap::value_type(
        "Clear", &SmoothlifeInstance::MessageClear));
  message_map_.insert(MessageMap::value_type(
        "Splat", &SmoothlifeInstance::MessageSplat));
  message_map_.insert(MessageMap::value_type(
        "Step", &SmoothlifeInstance::MessageStep));
  message_map_.insert(MessageMap::value_type(
        "Run", &SmoothlifeInstance::MessageRun));
}

void SmoothlifeInstance::DidChangeView(const pp::View& view) {
  if (!view_->DidChangeView(this, view)) {
    PostMessage(pp::Var(
        "ERROR DidChangeView failed. Could not bind graphics?"));
    return;
  }

  if (is_initial_view_change_) {
    ScheduleUpdate();
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

      if (left_down_) {
        pp::Point sim_point =
            view_->ScreenToSim(mouse_event.GetPosition(), sim_size_);
        EnqueueTask(MakeFunctionTask(&SmoothlifeThread::TaskDrawFilledCircle,
                                     sim_point.x(),
                                     sim_point.y(),
                                     10,
                                     1.0));
      }
      return true;
    }
    case PP_INPUTEVENT_TYPE_KEYUP:
      return true;
    case PP_INPUTEVENT_TYPE_KEYDOWN: {
      const uint32_t kKeyEnter = 0x0D;
      pp::KeyboardInputEvent key_event(event);
      if (key_event.GetKeyCode() == kKeyEnter) {
        if (!fullscreen_.IsFullscreen()) {
          fullscreen_.SetFullscreen(true);
        } else {
          fullscreen_.SetFullscreen(false);
        }
      }
      return true;
    }
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

void SmoothlifeInstance::MessageStep(const ParamList& params) {
  if (params.size() != 0)
    return;

  EnqueueTask(MakeFunctionTask(&SmoothlifeThread::TaskSetRunOptions,
                               kRunOptions_Step));

  step_cond_->Lock();
  step_cond_->Signal();
  step_cond_->Unlock();
}

void SmoothlifeInstance::MessageRun(const ParamList& params) {
  if (params.size() != 0)
    return;

  EnqueueTask(MakeFunctionTask(&SmoothlifeThread::TaskSetRunOptions,
                               kRunOptions_Continuous));

  step_cond_->Lock();
  step_cond_->Signal();
  step_cond_->Unlock();
}

void SmoothlifeInstance::EnqueueTask(Task* task) {
  ScopedLocker<TaskQueue> locker(*task_queue_);
  locker.object()->push_back(task);
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
  static PP_TimeTicks last_time = 0;
  if (last_time) {
    PP_TimeTicks this_time = GetTimeTicks();
    int num_frames;
    int* frames = frames_drawn_->Lock();
    num_frames = *frames;
    *frames = 0;
    frames_drawn_->Unlock();

    float fps = num_frames / (this_time - last_time);
    char buffer[20];
    sprintf(&buffer[0], "FPS: %.3f", fps);
    PostMessage(pp::Var(buffer));

    last_time = this_time;
  } else {
    last_time = GetTimeTicks();
  }
}
