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
#include "palette.h"
#include "simulation_config.h"
#include "simulation_thread.h"
#include "simulation_thread_options.h"
#include "smoother_config.h"
#include "task.h"
#include "view_base.h"

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
      fullscreen_(this),
      is_initial_view_change_(true) {
  // Request to receive input events.
  RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE | PP_INPUTEVENT_CLASS_KEYBOARD);
}

SmoothlifeInstance::~SmoothlifeInstance() {
  delete thread_;
  delete view_;
}

bool SmoothlifeInstance::Init(uint32_t argc, const char* argn[],
                              const char* argv[]) {
  glInitializePPAPI(pp::Module::Get()->get_browser_interface());
  InitMessageMap();
  SimulationConfig config;
  config.size = sim_size_;

  InitializerFactoryBase* initializer_factory;
  initializer_factory = new cpu::InitializerFactory(config.size);
  //initializer_factory = new gpu::InitializerFactory(config.size);

  SimulationThreadContext context;
  context.config = config;
  context.run_options = kRunOptions_Simulation;
  context.draw_options = kDrawOptions_Simulation;
  context.initializer_factory = initializer_factory;

  thread_ = new SimulationThread(context);
  view_ = initializer_factory->CreateView();

  ParseInitMessages(argc, argn, argv, &context);
  thread_->Start();

  return true;
}

void SmoothlifeInstance::ParseInitMessages(
    uint32_t argc,
    const char* argn[], const char* argv[],
    SimulationThreadContext* context) {
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
        "SetPalette", &SmoothlifeInstance::MessageSetPalette));
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
  message_map_.insert(MessageMap::value_type(
        "Screenshot", &SmoothlifeInstance::MessageScreenshot));
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
        thread_->EnqueueTask(MakeFunctionTask(
              &SimulationThread::TaskDrawFilledCircle,
              sim_point.x(),
              sim_point.y(),
              10,
              1.0));
      }
      return false;
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
  thread_->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskSetKernel,
                                        config));
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
  thread_->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskSetSmoother,
                                        config));
}

void SmoothlifeInstance::MessageSetPalette(const ParamList& params) {
  PaletteConfig config;
  if (params.size() % 2 != 1)
    return;

  config.repeating = static_cast<bool>(atoi(params[0].c_str()));

  for (int i = 1; i < params.size(); i += 2) {
    if (params[i].size() < 1)
      continue;

    const char* color_string = &params[i].c_str()[1];  // Skip the #
    uint32_t color = static_cast<uint32_t>(strtoul(color_string, NULL, 16));
    color |= 0xff000000;  // Set alpha to full.
    double stop = strtod(params[i + 1].c_str(), NULL) / 100.0;
    config.stops.push_back(ColorStop(color, stop));
  }
  thread_->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskSetPalette,
                                        config));
}

void SmoothlifeInstance::MessageClear(const ParamList& params) {
  if (params.size() != 1)
    return;

  double color = strtod(params[0].c_str(), NULL);
  thread_->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskClear, color));
}

void SmoothlifeInstance::MessageSplat(const ParamList& params) {
  if (params.size() != 0)
    return;

  thread_->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskSplat));
}

void SmoothlifeInstance::MessageSetRunOptions(const ParamList& params) {
  if (params.size() > 2)
    return;

  SimulationThreadRunOptions run_options = 0;
  for (int i = 0; i < params.size(); ++i) {
    if (params[i] == "simulation")
      run_options |= kRunOptions_Simulation;
    else if (params[i] == "noSimulation")
      run_options &= ~kRunOptions_Simulation;
    else if (params[i] == "pause")
      run_options |= kRunOptions_Pause;
    else if (params[i] == "run")
      run_options &= ~kRunOptions_Pause;
    else {
      printf("Unknown value %s for SetRunOptions, ignoring.\n",
          params[i].c_str());
      continue;
    }
  }

  thread_->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskSetRunOptions,
                               run_options));
  thread_->Step();
}

void SmoothlifeInstance::MessageSetDrawOptions(const ParamList& params) {
  if (params.size() != 1)
    return;

  SimulationThreadDrawOptions draw_options;
  if (params[0] == "simulation")
    draw_options = kDrawOptions_Simulation;
  else if (params[0] == "disc")
    draw_options = kDrawOptions_KernelDisc;
  else if (params[0] == "ring")
    draw_options = kDrawOptions_KernelRing;
  else if (params[0] == "smoother")
    draw_options = kDrawOptions_Smoother;
  else if (params[0] == "palette")
    draw_options = kDrawOptions_Palette;
  else {
    printf("Unknown value for SetDrawOptions, ignoring.\n");
    return;
  }

  thread_->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskSetDrawOptions,
                               draw_options));
}

void SmoothlifeInstance::MessageSetFullscreen(const ParamList& params) {
  if (params.size() != 1)
    return;

  fullscreen_.SetFullscreen(params[0] == "true");
}

void SmoothlifeInstance::MessageScreenshot(const ParamList& params) {
  if (params.size() != 0)
    return;

  thread_->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskScreenshot));
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
  static PP_TimeTicks first_time;
  static PP_TimeTicks last_time = 0;
  if (last_time) {
    PP_TimeTicks this_time = GetTimeTicks();
    int num_frames = thread_->GetFramesDrawnAndReset();
    float fps = num_frames / (this_time - last_time);
    char buffer[20];
    sprintf(&buffer[0], "FPS: %.3f", fps);
    PostMessage(pp::Var(buffer));

    last_time = this_time;
  } else {
    first_time = GetTimeTicks();
    last_time = GetTimeTicks();
  }
}
