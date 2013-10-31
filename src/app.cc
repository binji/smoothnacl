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

#include "app.h"

#include <sys/time.h>
#include <time.h>

#include <algorithm>
#include <string>

#include <ppapi/c/ppb_input_event.h>
#include <ppapi/c/pp_rect.h>
#include <ppapi/cpp/input_event.h>
#include <ppapi/cpp/point.h>
#include <ppapi/cpp/size.h>
#include <ppapi/cpp/var.h>
#include <ppapi/cpp/var_array.h>
#include <ppapi/cpp/var_dictionary.h>

#include "ppapi_simple/ps.h"
#include "ppapi_simple/ps_context_2d.h"
#include "ppapi_simple/ps_event.h"
#include "ppapi_simple/ps_interface.h"
#include "ppapi_simple/ps_main.h"

#include "simulation.h"

namespace {

const pp::Size kSimSize(512, 512);

#ifndef NDEBUG
bool IsPowerOf2(uint32_t x) {
  return x && (x & (x - 1)) == 0;
}
#endif

int TimevalToMs(struct timeval* t) {
    return (t->tv_sec * 1000 + t->tv_usec / 1000);
}

int TimeDeltaMs(struct timeval* start, struct timeval* end) {
  return TimevalToMs(end) - TimevalToMs(start);
}

}  // namespace


App::App() :
    context_(PSContext2DAllocate(PP_IMAGEDATAFORMAT_BGRA_PREMUL)),
    simulation_config_(kSimSize),
    simulation_(simulation_config_),
    palette_(palette_config_),
    screen_to_sim_scale_(1),
    screen_to_sim_x_offset_(0),
    screen_to_sim_y_offset_(0),
    mouse_down_(false),
    brush_radius_(10),
    brush_color_(1) {
}

App::~App() {
  PSContext2DFree(context_);
}

void App::Run() {
  PSEventSetFilter(PSE_ALL);

  const int kFpsUpdateMs = 1000;
  int frames_drawn = 0;
  struct timeval last_frame_time;
  gettimeofday(&last_frame_time, NULL);

  while (true) {
    PSEvent* event;

    while ((event = PSEventTryAcquire()) != NULL) {
      HandleEvent(event);
      PSEventRelease(event);
    }

    Update();

    PSContext2DGetBuffer(context_);
    if (context_->data) {
      Render();
      frames_drawn++;
      PSContext2DSwapBuffer(context_);
    }

    struct timeval current_frame_time;
    gettimeofday(&current_frame_time, NULL);

    int diff_ms = TimeDeltaMs(&last_frame_time, &current_frame_time);
    if (diff_ms > kFpsUpdateMs) {
      double fps = static_cast<double>(frames_drawn * 1000) / diff_ms;
      pp::Var fps_var(fps);
      PSInterfaceMessaging()->PostMessage(PSGetInstanceId(), fps_var.pp_var());
      frames_drawn = 0;
      last_frame_time = current_frame_time;
    }
  }
}

void App::HandleEvent(PSEvent* event) {
  // Listen for view changes, then pass them on to the context.
  if (event->type == PSE_INSTANCE_DIDCHANGEVIEW)
    HandleDidChangeView(event);

  if (PSContext2DHandleEvent(context_, event))
    return;

  if (event->type == PSE_INSTANCE_HANDLEINPUT) {
    pp::InputEvent input(event->as_resource);
    HandleInput(input);
  } else if (event->type == PSE_INSTANCE_HANDLEMESSAGE) {
    pp::Var var(event->as_var);
    HandleMessage(var);
  }
}

void App::HandleDidChangeView(PSEvent* event) {
  struct PP_Rect rect;
  PSInterfaceView()->GetRect(event->as_resource, &rect);

  // Update cached screen_to_sim_* vars.
  // Keep the aspect ratio.
  int sim_width = simulation_.size().width();
  int sim_height = simulation_.size().height();
  int screen_width = rect.size.width;
  int screen_height = rect.size.height;
  screen_to_sim_scale_ = std::max(
      static_cast<double>(sim_width) / screen_width,
      static_cast<double>(sim_height) / screen_height);
  screen_to_sim_x_offset_ =
      static_cast<int>((screen_width - sim_width / screen_to_sim_scale_) / 2);
  screen_to_sim_y_offset_ =
      static_cast<int>((screen_height - sim_height / screen_to_sim_scale_) / 2);

  printf("HandleDidChangeView: scale: %f, xoff: %f, yoff: %f\n",
      screen_to_sim_scale_, screen_to_sim_x_offset_, screen_to_sim_y_offset_);
}

pp::Point App::ScreenToSim(const pp::Point& p) const {
  double x_offset = screen_to_sim_x_offset_;
  double y_offset = screen_to_sim_y_offset_;
  double scale = screen_to_sim_scale_;
  return pp::Point(static_cast<int>((p.x() - x_offset) * scale),
                   static_cast<int>((p.y() - y_offset) * scale));
}

void App::HandleInput(const pp::InputEvent& event) {
  if (event.GetType() == PP_INPUTEVENT_TYPE_MOUSEUP ||
      event.GetType() == PP_INPUTEVENT_TYPE_MOUSEDOWN ||
      event.GetType() == PP_INPUTEVENT_TYPE_MOUSEMOVE) {
    pp::MouseInputEvent mouse_event(event);

    if (event.GetType() != PP_INPUTEVENT_TYPE_MOUSEMOVE &&
        mouse_event.GetButton() == PP_INPUTEVENT_MOUSEBUTTON_LEFT) {
      mouse_down_ = event.GetType() == PP_INPUTEVENT_TYPE_MOUSEDOWN;
    }

    mouse_point_ = ScreenToSim(mouse_event.GetPosition());
  }
}

void App::HandleMessage(const pp::Var& var) {
  if (!var.is_dictionary()) {
    printf("Not dictionary. Ignoring.\n");
    return;
  }

  pp::VarDictionary dictionary(var);
  std::string cmd = dictionary.Get("cmd").AsString();

  if (cmd == "clear") {
    double color = dictionary.Get("color").AsDouble();
    printf("clear{color: %f}\n", color);
    simulation_.Clear(color);
  } else if (cmd == "setBrush") {
    brush_radius_ = dictionary.Get("radius").AsDouble();
    brush_color_ = dictionary.Get("color").AsDouble();
    printf("setBrush{radius: %f, color: %f}\n", brush_radius_, brush_color_);
  } else if (cmd == "setDrawOptions") {
  } else if (cmd == "setKernel") {
    KernelConfig config;
    config.disc_radius = dictionary.Get("discRadius").AsDouble();
    config.ring_radius = dictionary.Get("ringRadius").AsDouble();
    config.blend_radius = dictionary.Get("blendRadius").AsDouble();
    printf("setKernel{discRadius: %f, ringRadius: %f, blendRadius: %f}\n",
           config.disc_radius, config.ring_radius, config.blend_radius);
    simulation_.SetKernel(config);
  } else if (cmd == "setPalette") {
    PaletteConfig config;
    config.repeating = dictionary.Get("repeating").AsBool();
    pp::VarArray colors(dictionary.Get("colors"));
    pp::VarArray stops(dictionary.Get("stops"));
    uint32_t length = std::min(colors.GetLength(), stops.GetLength());
    printf("setPalette{repeating: %d, colors: [", config.repeating);
    for (uint32_t i = 0; i < length; ++i) {
      double stop = stops.Get(i).AsDouble() / 100;
      std::string color_string = colors.Get(i).AsString();
      if (color_string.length() < 1)
        continue;
      uint32_t color =
          static_cast<uint32_t>(strtoul(&color_string.c_str()[1], NULL, 16));
      color |= 0xff000000;  // Set alpha to full.
      printf("[%x,%f], ", color, stop);
      config.stops.push_back(ColorStop(color, stop));
    }
    printf("]}\n");
    palette_.SetConfig(config);
  } else if (cmd == "setRunOptions") {
  } else if (cmd == "setSmoother") {
    SmootherConfig config;
    config.timestep.type =
        static_cast<Timestep>(dictionary.Get("type").AsInt());
    config.timestep.dt = dictionary.Get("dt").AsDouble();
    config.b1 = dictionary.Get("b1").AsDouble();
    config.d1 = dictionary.Get("d1").AsDouble();
    config.b2 = dictionary.Get("b2").AsDouble();
    config.d2 = dictionary.Get("d2").AsDouble();
    config.mode = static_cast<SigmoidMode>(dictionary.Get("mode").AsInt());
    config.sigmoid = static_cast<Sigmoid>(dictionary.Get("sigmoid").AsInt());
    config.mix = static_cast<Sigmoid>(dictionary.Get("mix").AsInt());
    config.sn = dictionary.Get("sn").AsDouble();
    config.sm = dictionary.Get("sm").AsDouble();
    printf("setSmoother{type: %d, dt: %f, b1: %f, d1: %f, b2: %f, d2: %f"
           ", mode: %d, sigmoid: %d, mix: %d, sn: %f, sm: %f}\n",
           config.timestep.type, config.timestep.dt,
           config.b1, config.d1, config.b2, config.d2,
           config.mode, config.sigmoid, config.mix,
           config.sn, config.sm);
    simulation_.SetSmoother(config);
  } else if (cmd == "splat") {
    printf("splat{}\n");
    simulation_.Splat();
  } else {
    printf("Unknown command: %s\n", cmd.c_str());
  }
}

void App::Update() {
  if (mouse_down_) {
    simulation_.DrawFilledCircle(mouse_point_.x(), mouse_point_.y(),
                                 brush_radius_, brush_color_);
  }

  simulation_.Step();
}

void App::Render() {
  uint32_t* pixels = context_->data;
  if (!pixels) {
    printf("No pixels.\n");
    return;
  }

  const AlignedReals& buffer = simulation_.buffer();
  int buffer_width = buffer.size().width();
  int buffer_height = buffer.size().height();
  assert(IsPowerOf2(buffer_width));
  assert(IsPowerOf2(buffer_height));

  int screen_width = context_->width;
  int screen_height = context_->height;
  double x_offset = screen_to_sim_x_offset_;
  double y_offset = screen_to_sim_y_offset_;
  double scale = screen_to_sim_scale_;

  for (int y = 0; y < screen_height; ++y) {
    int buffer_y = static_cast<int>((y - y_offset) * scale);
    buffer_y = (buffer_y + buffer_height) & (buffer_height - 1);
    for (int x = 0; x < screen_width; ++x) {
      int buffer_x = static_cast<int>((x - x_offset) * scale);
      buffer_x = (buffer_x + buffer_width) & (buffer_width - 1);
      double color_value = buffer[buffer_y * buffer_width + buffer_x];
      uint32_t color = palette_.GetColor(color_value);
      pixels[y * screen_width + x] = color;
    }
  }
}
