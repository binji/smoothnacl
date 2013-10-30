// Copyright 2013 Ben Smith. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "instance.h"
#include <algorithm>
#include <functional>
#include <stdio.h>
#include <string>
#include <ppapi/c/pp_time.h>
#include <ppapi/cpp/core.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/input_event.h>
#include <ppapi/cpp/var.h>
#include <ppapi/lib/gl/gles2/gl2ext_ppapi.h>

#include "cpu/initializer_factory.h"
#include "gpu/initializer_factory.h"
#include "messages.h"
#include "simulation_config.h"
#include "simulation_thread.h"
#include "simulation_thread_options.h"
#include "task.h"
#include "view_base.h"

namespace {

const int kUpdateInterval = 1000;
const pp::Size kSimSize(512, 512);

double GetTimeTicks() {
  return pp::Module::Get()->core()->GetTimeTicks();
}

}  // namespace


Instance::Instance(PP_Instance instance)
    : pp::Instance(instance),
      factory_(this),
      view_(NULL),
      thread_(NULL),
      fullscreen_(this),
      is_initial_view_change_(true),
      brush_radius_(10),
      brush_color_(1.0) {
  // Request to receive input events.
  RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE | PP_INPUTEVENT_CLASS_KEYBOARD);
}

Instance::~Instance() {
  delete thread_;
  delete view_;
}

bool Instance::Init(uint32_t argc, const char* argn[],
                              const char* argv[]) {
  glInitializePPAPI(pp::Module::Get()->get_browser_interface());
  SimulationConfig config;
  config.size = kSimSize;

  InitializerFactoryBase* initializer_factory;
  initializer_factory = new cpu::InitializerFactory(config.size);
  //initializer_factory = new gpu::InitializerFactory(config.size);

  SimulationThreadContext context;
  context.config = config;
  context.run_options = kRunOptions_Simulation;
  context.draw_options = kDrawOptions_Simulation;
  context.initializer_factory = initializer_factory;

  thread_ = new SimulationThread(this, context);
  view_ = initializer_factory->CreateView();

  InitMessageMap();
  ParseInitMessages(argc, argn, argv);
  thread_->Start();

  return true;
}

void Instance::ParseInitMessages(
    uint32_t argc, const char* argn[], const char* argv[]) {
  for (uint32_t i = 0; i < argc; ++i) {
    if (strncmp(argn[i], "msg", 3) == 0) {
      HandleMessage(pp::Var(argv[i]));
    }
  }
}

void Instance::InitMessageMap() {
  using namespace std::placeholders;

#define MESSAGE(NAME, ...) \
  message_handler_.AddHandler(#NAME, std::bind(&msg::NAME, __VA_ARGS__));

  MESSAGE(Clear, thread_, _1);
  MESSAGE(GetBuffer, thread_, _1);
  MESSAGE(SetBrush, _1, &brush_radius_, &brush_color_);
  MESSAGE(SetDrawOptions, thread_, _1);
  MESSAGE(SetFullscreen, _1, &fullscreen_);
  MESSAGE(SetKernel, thread_, _1);
  MESSAGE(SetPalette, thread_, _1);
  MESSAGE(SetRunOptions, thread_, _1);
  MESSAGE(SetSmoother, thread_, _1);
  MESSAGE(Splat, thread_, _1);

#undef M
}

void Instance::DidChangeView(const pp::View& view) {
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

bool Instance::HandleInputEvent(const pp::InputEvent& event) {
  static bool left_down_ = false;

  switch (event.GetType()) {
    case PP_INPUTEVENT_TYPE_MOUSEUP: {
      pp::MouseInputEvent mouse_event(event);
      if (mouse_event.GetButton() == PP_INPUTEVENT_MOUSEBUTTON_LEFT)
        left_down_ = false;
      break;
    }

    case PP_INPUTEVENT_TYPE_MOUSEDOWN: {
      pp::MouseInputEvent mouse_event(event);
      if (mouse_event.GetButton() == PP_INPUTEVENT_MOUSEBUTTON_LEFT)
        left_down_ = true;
      // fall through
    }

    case PP_INPUTEVENT_TYPE_MOUSEMOVE: {
      pp::MouseInputEvent mouse_event(event);

      if (left_down_) {
        pp::Point sim_point =
            view_->ScreenToSim(mouse_event.GetPosition(), kSimSize);
        thread_->EnqueueTask(MakeFunctionTask(
            &SimulationThread::TaskDrawFilledCircle,
            sim_point.x(),
            sim_point.y(),
            brush_radius_,
            brush_color_));
      }
      return false;
    }
    default:
    case PP_INPUTEVENT_TYPE_KEYUP:
    case PP_INPUTEVENT_TYPE_KEYDOWN:
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

void Instance::HandleMessage(const pp::Var& var_message) {
  if (!var_message.is_string())
    return;
  std::string message = var_message.AsString();
  message_handler_.HandleMessage(message);
}

void Instance::ScheduleUpdate() {
  pp::Module::Get()->core()->CallOnMainThread(
      kUpdateInterval,
      factory_.NewCallback(&Instance::UpdateCallback));
}

void Instance::UpdateCallback(int32_t result) {
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
