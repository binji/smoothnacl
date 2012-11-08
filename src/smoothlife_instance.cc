// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "smoothlife_instance.h"
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <ppapi/c/pp_time.h>
#include <ppapi/cpp/core.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/input_event.h>
#include <ppapi/cpp/var.h>
#include <ppapi/lib/gl/gles2/gl2ext_ppapi.h>

#include "cpu/initializer_factory.h"
#include "gpu/initializer_factory.h"
#include "kernel_config.h"
#include "simulation_config.h"
#include "smoother_config.h"
#include "task.h"
#include "thread.h"
#include "thread_options.h"
#include "view_base.h"

namespace {

const int kUpdateInterval = 1000;
const int kMaxTaskQueueSize = 25;

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
      task_queue_(NULL),
      frames_drawn_(NULL),
      fullscreen_(this),
      is_initial_view_change_(true) {
  // Request to receive input events.
  RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE | PP_INPUTEVENT_CLASS_KEYBOARD);
}

SmoothlifeInstance::~SmoothlifeInstance() {
  delete task_queue_;
  delete thread_;
  delete view_;
}

bool SmoothlifeInstance::Init(uint32_t argc, const char* argn[],
                              const char* argv[]) {
  glInitializePPAPI(pp::Module::Get()->get_browser_interface());
  InitMessageMap();
  SimulationConfig config;
  config.size = sim_size_;

  task_queue_ = new LockedObject<TaskQueue>(new TaskQueue);
  frames_drawn_ = new LockedObject<int>(new int(0));
  //initializer_factory_ = new cpu::InitializerFactory(config.size);
  initializer_factory_ = new gpu::InitializerFactory(config.size);
  step_cond_ = new CondVar;

  ThreadContext context;
  context.config = config;
  context.run_options = kRunOptions_Continuous;
  context.draw_options = kDrawOptions_Simulation;
  context.queue = task_queue_;
  context.frames_drawn = frames_drawn_;
  context.step_cond = step_cond_;
  context.initializer_factory = initializer_factory_;

  ParseInitMessages(argc, argn, argv, &context);

  thread_ = new Thread(context);
  view_ = initializer_factory_->CreateView();

  return true;
}

void SmoothlifeInstance::ParseInitMessages(
    uint32_t argc,
    const char* argn[], const char* argv[],
    ThreadContext* context) {
  for (uint32_t i = 0; i < argc; ++i) {
    if (strncmp(argn[i], "msg", 3) == 0) {
      HandleMessage(pp::Var(argv[i]));
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
        "SetRunOptions", &SmoothlifeInstance::MessageSetRunOptions));
  message_map_.insert(MessageMap::value_type(
        "SetDrawOptions", &SmoothlifeInstance::MessageSetDrawOptions));
  message_map_.insert(MessageMap::value_type(
        "SetFullscreen", &SmoothlifeInstance::MessageSetFullscreen));
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
        EnqueueTask(MakeFunctionTask(
              &Thread::TaskDrawFilledCircle,
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
      const uint32_t kKeyEscape = 0x1B;
      pp::KeyboardInputEvent key_event(event);
      if (key_event.GetKeyCode() == kKeyEnter) {
        if (!fullscreen_.IsFullscreen()) {
          fullscreen_.SetFullscreen(true);
        } else {
          fullscreen_.SetFullscreen(false);
        }
      } else if (key_event.GetKeyCode() == kKeyEscape) {
        fullscreen_.SetFullscreen(false);
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
  if (func_iter != message_map_.end()) {
    printf("Got message: %s\n", message.c_str());
    (this->*func_iter->second)(params);
  } else {
    printf("Unknown message: %s\n", message.c_str());
  }
}

void SmoothlifeInstance::MessageSetKernel(const ParamList& params) {
  if (params.size() != 3)
    return;

  KernelConfig config;
  config.disc_radius = strtod(params[0].c_str(), NULL);
  config.ring_radius = strtod(params[1].c_str(), NULL);
  config.blend_radius = strtod(params[2].c_str(), NULL);
  EnqueueTask(MakeFunctionTask(&Thread::TaskSetKernel, config));
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
  EnqueueTask(MakeFunctionTask(&Thread::TaskSetSmoother, config));
}

void SmoothlifeInstance::MessageClear(const ParamList& params) {
  if (params.size() != 1)
    return;

  double color = strtod(params[0].c_str(), NULL);
  EnqueueTask(MakeFunctionTask(&Thread::TaskClear, color));
}

void SmoothlifeInstance::MessageSplat(const ParamList& params) {
  if (params.size() != 0)
    return;

  EnqueueTask(MakeFunctionTask(&Thread::TaskSplat));
}

void SmoothlifeInstance::MessageSetRunOptions(const ParamList& params) {
  if (params.size() != 1)
    return;

  ThreadRunOptions run_options;
  if (params[0] == "step")
    run_options = kRunOptions_Step;
  else if (params[0] == "continuous")
    run_options = kRunOptions_Continuous;
  else if (params[0] == "none")
    run_options = kRunOptions_None;
  else {
    printf("Unknown value for SetRunOptions, ignoring.\n");
    return;
  }

  EnqueueTask(MakeFunctionTask(&Thread::TaskSetRunOptions, run_options));

  step_cond_->Lock();
  step_cond_->Signal();
  step_cond_->Unlock();
}

void SmoothlifeInstance::MessageSetDrawOptions(const ParamList& params) {
  if (params.size() != 1)
    return;

  ThreadDrawOptions draw_options;
  if (params[0] == "simulation")
    draw_options = kDrawOptions_Simulation;
  else if (params[0] == "kernelDisc")
    draw_options = kDrawOptions_KernelDisc;
  else if (params[0] == "kernelRing")
    draw_options = kDrawOptions_KernelRing;
  else if (params[0] == "smoother")
    draw_options = kDrawOptions_Smoother;
  else {
    printf("Unknown value for SetDrawOptions, ignoring.\n");
    return;
  }

  EnqueueTask(MakeFunctionTask(&Thread::TaskSetDrawOptions, draw_options));
}

void SmoothlifeInstance::MessageSetFullscreen(const ParamList& params) {
  if (params.size() != 1)
    return;

  fullscreen_.SetFullscreen(params[0] == "true");
}

void SmoothlifeInstance::EnqueueTask(Task* task) {
  ScopedLocker<TaskQueue> locker(*task_queue_);
  if (locker.object()->size() > kMaxTaskQueueSize) {
    printf("Task queue is full, dropping message.\n");
    return;
  }

  locker.object()->push_back(std::shared_ptr<Task>(task));
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
