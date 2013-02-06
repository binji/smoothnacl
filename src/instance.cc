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
#include <array>
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
#include "image_operation.h"
#include "kernel_config.h"
#include "palette.h"
#include "screenshot_config.h"
#include "simulation_config.h"
#include "simulation_thread.h"
#include "simulation_thread_options.h"
#include "smoother_config.h"
#include "task.h"
#include "view_base.h"

namespace {

const int kUpdateInterval = 1000;
const double kMaxBrushRadius = 100.0;

double GetTimeTicks() {
  return pp::Module::Get()->core()->GetTimeTicks();
}

std::vector<std::string> Split(const std::string& s, char delim) {
  const char kWhitespace[] = " \t\n";
  std::vector<std::string> components;
  size_t start = 0;
  size_t delim_pos;
  do {
    start = s.find_first_not_of(kWhitespace, start);
    if (start == std::string::npos)
      break;

    delim_pos = s.find(delim, start);
    size_t end = s.find_last_not_of(kWhitespace, delim_pos);
    if (end != delim_pos)
      end++;

    components.push_back(s.substr(start, end - start));
    start = delim_pos + 1;
  } while (delim_pos != std::string::npos);

  return components;
}

}  // namespace


Instance::Instance(PP_Instance instance)
    : pp::Instance(instance),
      factory_(this),
      view_(NULL),
      thread_(NULL),
      sim_size_(512, 512),
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
  InitMessageMap();
  SimulationConfig config;
  config.size = sim_size_;

  InitializerFactoryBase* initializer_factory;
  initializer_factory = new cpu::InitializerFactory(this, config.size);
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

void Instance::ParseInitMessages(
    uint32_t argc,
    const char* argn[], const char* argv[],
    SimulationThreadContext* context) {
  for (uint32_t i = 0; i < argc; ++i) {
    if (strncmp(argn[i], "msg", 3) == 0) {
      HandleMessage(pp::Var(argv[i]));
    }
  }
}

void Instance::InitMessageMap() {
  message_map_.insert(MessageMap::value_type(
        "SetKernel", &Instance::MessageSetKernel));
  message_map_.insert(MessageMap::value_type(
        "SetSmoother", &Instance::MessageSetSmoother));
  message_map_.insert(MessageMap::value_type(
        "SetPalette", &Instance::MessageSetPalette));
  message_map_.insert(MessageMap::value_type(
        "Clear", &Instance::MessageClear));
  message_map_.insert(MessageMap::value_type(
        "Splat", &Instance::MessageSplat));
  message_map_.insert(MessageMap::value_type(
        "SetRunOptions", &Instance::MessageSetRunOptions));
  message_map_.insert(MessageMap::value_type(
        "SetDrawOptions", &Instance::MessageSetDrawOptions));
  message_map_.insert(MessageMap::value_type(
        "SetFullscreen", &Instance::MessageSetFullscreen));
  message_map_.insert(MessageMap::value_type(
        "Screenshot", &Instance::MessageScreenshot));
  message_map_.insert(MessageMap::value_type(
        "SetBrush", &Instance::MessageSetBrush));
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
              brush_radius_,
              brush_color_));
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

void Instance::HandleMessage(const pp::Var& var_message) {
  if (!var_message.is_string())
    return;
  std::string message = var_message.AsString();
  size_t colon = message.find(':');

  std::string function = message.substr(0, colon);
  std::vector<std::string> params;

  if (colon != std::string::npos) {
    params = Split(message.substr(colon + 1), ',');
  }

  MessageMap::iterator func_iter = message_map_.find(function);
  if (func_iter != message_map_.end()) {
    printf("Got message: %s\n", message.c_str());
    (this->*func_iter->second)(params);
  } else {
    printf("Unknown message: %s\n", message.c_str());
  }
}

void Instance::MessageSetKernel(const ParamList& params) {
  if (params.size() != 3)
    return;

  KernelConfig config;
  config.disc_radius = strtod(params[0].c_str(), NULL);
  config.ring_radius = strtod(params[1].c_str(), NULL);
  config.blend_radius = strtod(params[2].c_str(), NULL);
  thread_->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskSetKernel,
                                        config));
}

void Instance::MessageSetSmoother(const ParamList& params) {
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

void Instance::MessageSetPalette(const ParamList& params) {
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

void Instance::MessageClear(const ParamList& params) {
  if (params.size() != 1)
    return;

  double color = strtod(params[0].c_str(), NULL);
  thread_->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskClear, color));
}

void Instance::MessageSplat(const ParamList& params) {
  if (params.size() != 0)
    return;

  thread_->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskSplat));
}

void Instance::MessageSetRunOptions(const ParamList& params) {
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

void Instance::MessageSetDrawOptions(const ParamList& params) {
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

void Instance::MessageSetFullscreen(const ParamList& params) {
  if (params.size() != 1)
    return;

  fullscreen_.SetFullscreen(params[0] == "true");
}

void Instance::MessageScreenshot(const ParamList& params) {
  if (params.size() < 2)
    return;

  ScreenshotConfig config;
  config.request_id = atoi(params[0].c_str());

  config.file_format = params[1];
  static const std::array<std::string, 2> valid_file_formats = {
    "PNG", "JPEG" };
  const std::string* found = std::find(valid_file_formats.begin(),
                                       valid_file_formats.end(),
                                       config.file_format);
  if (found == valid_file_formats.end()) {
    printf("Unknown file format for Screenshot, ignoring.\n");
    return;
  }

  for (int i = 2; i < params.size(); ++i) {
    const std::string& param = params[i];
    // Split each param at spaces.
    std::vector<std::string> operation_params = Split(param, ' ');

    if (operation_params.size() == 0) {
      printf("Ignoring empty operation.\n");
      continue;
    }

    std::string operation = operation_params[0];
    operation_params.erase(operation_params.begin());

    ImageOperation* op = NULL;
    if (operation == "reduce") {
      if (operation_params.size() != 1)
        continue;

      int max_length = atoi(operation_params[0].c_str());
      op = new ReduceImageOperation(max_length);
    } else if (operation == "crop") {
      if (operation_params.size() != 3)
        continue;

      double x_scale = strtod(operation_params[0].c_str(), NULL);
      double y_scale = strtod(operation_params[1].c_str(), NULL);
      int max_length = atoi(operation_params[2].c_str());
      op = new CropImageOperation(x_scale, y_scale, max_length);
    } else if (operation == "brightness_contrast") {
      if (operation_params.size() != 2)
        continue;

      double brightness_shift = strtod(operation_params[0].c_str(), NULL);
      double contrast_factor = strtod(operation_params[1].c_str(), NULL);
      op = new BrightnessContrastImageOperation(brightness_shift,
                                                contrast_factor);
    } else {
      printf("Unknown operation %s, ignoring.\n", operation.c_str());
    }

    if (op)
      config.operations.push_back(ScreenshotConfig::OperationPtr(op));
  }

  thread_->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskScreenshot,
                                        config));
}

void Instance::MessageSetBrush(const ParamList& params) {
  if (params.size() < 2)
    return;

  brush_radius_ = strtod(params[0].c_str(), NULL);
  brush_color_ = strtod(params[1].c_str(), NULL);

  // Clamp values.
  brush_radius_ = std::max(std::min(brush_radius_, kMaxBrushRadius), 0.0);
  brush_color_ = std::max(std::min(brush_color_, 1.0), 0.0);
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
