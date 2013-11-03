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
#include "timer.h"

namespace {

//const pp::Size kSimSize(256, 256);
//const pp::Size kSimSize(384, 384);
const pp::Size kSimSize(512, 512);

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
    scale_numer_(1),
    scale_denom_(1),
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

    TIME(Update());

    PSContext2DGetBuffer(context_);
    if (context_->data) {
      TIME(Render());
      frames_drawn++;
      TIME(PSContext2DSwapBuffer(context_));
    }

    struct timeval current_frame_time;
    gettimeofday(&current_frame_time, NULL);

    int diff_ms = TimeDeltaMs(&last_frame_time, &current_frame_time);
    if (diff_ms > kFpsUpdateMs) {
      real fps = static_cast<real>(frames_drawn * 1000) / diff_ms;
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
  UpdateScreenScale(rect.size.width, rect.size.height);
}

void App::UpdateScreenScale(int screen_width, int screen_height) {
  // Update scale_{numer,denom}_ vars.
  // Keep the aspect ratio, and wrap in the longer dimension.
  int buffer_width = simulation_.size().width();
  int buffer_height = simulation_.size().height();

  if (buffer_width * screen_height > buffer_height * screen_width) {
    // tall
    scale_numer_ = buffer_width;
    scale_denom_ = screen_width;
  } else {
    // wide
    scale_numer_ = buffer_height;
    scale_denom_ = screen_height;
  }

  printf("UpdateScreenScale: scale: %d/%d\n", scale_numer_, scale_denom_);
}

pp::Point App::ScreenToSim(const pp::Point& p) const {
  return pp::Point(p.x() * scale_numer_ / scale_denom_,
                   p.y() * scale_numer_ / scale_denom_);
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
    real color = dictionary.Get("color").AsDouble();
    printf("clear{color: %f}\n", color);
    simulation_.Clear(color);
  } else if (cmd == "setSize") {
    int size = dictionary.Get("size").AsInt();
    printf("setSize{size: %d}\n", size);
    if (size != 256 && size != 384 && size != 512) {
      printf("  invalid size, ignoring.\n");
    }
    simulation_.SetSize(pp::Size(size, size));
    UpdateScreenScale(context_->width, context_->height);
  } else if (cmd == "setBrush") {
    brush_radius_ = dictionary.Get("radius").AsDouble();
    brush_color_ = dictionary.Get("color").AsDouble();
    printf("setBrush{radius: %f, color: %f}\n", brush_radius_, brush_color_);
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
      real stop = stops.Get(i).AsDouble() / 100;
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
  int screen_width = context_->width;
  int screen_height = context_->height;

  int by = 0;
  int x_accum = 0;
  int y_accum = 0;
  int no_wrap_width = buffer_width * scale_denom_ / scale_numer_;

  const real* row_start = buffer.data();
  const real* src = row_start;
  uint32_t* dst = pixels;
  uint32_t color = palette_.GetColor(*src);
  for (int sy = 0; sy < screen_height; ++sy) {
    int row_count = screen_width;

    while (row_count > 0) {
      int no_wrap_count = std::min(row_count, no_wrap_width);
      for (int x = 0; x < no_wrap_count; ++x) {
        *dst++ = color;
        x_accum += scale_numer_;
        while (x_accum >= scale_denom_) {
          x_accum -= scale_denom_;
          color = palette_.GetColor(*src++);
        }
      }
      row_count -= no_wrap_count;
      src = row_start;
    }

    y_accum += scale_numer_;
    while (y_accum >= scale_denom_) {
      y_accum -= scale_denom_;
      if (++by == buffer_height) {
        by = 0;
        src = row_start = buffer.data();
      } else {
        row_start += buffer_width;
        src = row_start;
      }
      color = palette_.GetColor(*src);
    }
  }
}
