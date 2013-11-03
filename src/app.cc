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

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>

#include <algorithm>
#include <string>

#include <ppapi/c/pp_rect.h>
#include <ppapi/c/ppb_image_data.h>
#include <ppapi/c/ppb_input_event.h>
#include <ppapi/cpp/graphics_2d.h>
#include <ppapi/cpp/image_data.h>
#include <ppapi/cpp/input_event.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/point.h>
#include <ppapi/cpp/size.h>
#include <ppapi/cpp/var.h>
#include <ppapi/cpp/var_array.h>
#include <ppapi/cpp/var_dictionary.h>
#include <ppapi/utility/completion_callback_factory.h>

#include "palette.h"
#include "simulation.h"
#include "simulation_config.h"

#ifdef WIN32
#undef PostMessage
// Allow 'this' in initializer list
#pragma warning(disable : 4355)
#endif

namespace {

const int kDefaultThreadCount = 1;
const int kMaxThreadCount = 32;

//const pp::Size kSimSize(256, 256);
//const pp::Size kSimSize(384, 384);
const pp::Size kSimSize(512, 512);
const int kFpsUpdateMs = 1000;

int TimevalToMs(struct timeval* t) {
    return (t->tv_sec * 1000 + t->tv_usec / 1000);
}

int TimeDeltaMs(struct timeval* start, struct timeval* end) {
  return TimevalToMs(end) - TimevalToMs(start);
}

}  // namespace

class Instance : public pp::Instance {
 public:
  explicit Instance(PP_Instance instance)
      : pp::Instance(instance),
        callback_factory_(this),
        simulation_config_(kDefaultThreadCount, kSimSize),
        simulation_(simulation_config_),
        palette_(palette_config_),
        scale_numer_(1),
        scale_denom_(1),
        mouse_down_(false),
        brush_radius_(10),
        brush_color_(1),
        frames_drawn_(0) {}

  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]) {
    RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE);
    gettimeofday(&last_frame_time_, NULL);
    return true;
  }

  virtual void DidChangeView(const pp::View& view) {
    context_size_ = view.GetRect().size();
    if (!CreateContext())
      return;

    UpdateScreenScale();

    // When flush_context_ is null, it means there is no Flush callback in
    // flight. This may have happened if the context was not created
    // successfully, or if this is the first call to DidChangeView (when the
    // module first starts). In either case, start the main loop.
    if (flush_context_.is_null())
      MainLoop(0);
  }

  virtual bool HandleInputEvent(const pp::InputEvent& event) {
    if (event.GetType() == PP_INPUTEVENT_TYPE_MOUSEUP ||
        event.GetType() == PP_INPUTEVENT_TYPE_MOUSEDOWN ||
        event.GetType() == PP_INPUTEVENT_TYPE_MOUSEMOVE) {
      pp::MouseInputEvent mouse_event(event);

      if (event.GetType() != PP_INPUTEVENT_TYPE_MOUSEMOVE &&
          mouse_event.GetButton() == PP_INPUTEVENT_MOUSEBUTTON_LEFT) {
        mouse_down_ = event.GetType() == PP_INPUTEVENT_TYPE_MOUSEDOWN;
      }

      mouse_point_ = ScreenToSim(mouse_event.GetPosition());
      return true;
    }
    return false;
  }

  pp::Point ScreenToSim(const pp::Point& p) const {
    return pp::Point(p.x() * scale_numer_ / scale_denom_,
                     p.y() * scale_numer_ / scale_denom_);
  }

  virtual void HandleMessage(const pp::Var& var) {
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
        printf("  invalid size (%d), ignoring.\n", size);
        return;
      }
      simulation_.SetSize(pp::Size(size, size));
      UpdateScreenScale();
    } else if (cmd == "setThreadCount") {
#ifdef USE_THREADS
      int thread_count = dictionary.Get("threadCount").AsInt();
      printf("setThreadCount{threadCount: %d}\n", thread_count);
      if (thread_count < 1 || thread_count > kMaxThreadCount) {
        printf("  invalid thread count (%d), ignoring.\n", thread_count);
        return;
      }
      simulation_.SetThreadCount(thread_count);
#else
      printf("threads disabled, ignoring message.\n");
#endif
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

 private:
  bool CreateContext() {
    const bool kIsAlwaysOpaque = true;
    context_ = pp::Graphics2D(this, context_size_, kIsAlwaysOpaque);
    if (!BindGraphics(context_)) {
      fprintf(stderr, "Unable to bind 2d context!\n");
      context_ = pp::Graphics2D();
      return false;
    }
    return true;
  }

  void UpdateScreenScale() {
    // Update scale_{numer,denom}_ vars.
    // Keep the aspect ratio, and wrap in the longer dimension.
    int screen_width = context_size_.width();
    int screen_height = context_size_.height();
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

  void Update() {
    if (mouse_down_) {
      simulation_.DrawFilledCircle(mouse_point_.x(), mouse_point_.y(),
                                   brush_radius_, brush_color_);
    }

    simulation_.Step();
  }

  void Render() {
    PP_ImageDataFormat format = pp::ImageData::GetNativeImageDataFormat();
    const bool kDontInitToZero = false;
    pp::ImageData image_data(this, format, context_size_, kDontInitToZero);
    uint32_t* pixels = static_cast<uint32_t*>(image_data.data());
    if (!pixels) {
      printf("No pixels.\n");
      return;
    }

    const AlignedReals& buffer = simulation_.buffer();
    int screen_width = image_data.size().width();
    int screen_height = image_data.size().height();
    int buffer_width = buffer.size().width();
    int buffer_height = buffer.size().height();

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
    context_.ReplaceContents(&image_data);
  }

  void MainLoop(int32_t) {
    if (context_.is_null()) {
      // The current Graphics2D context is null, so updating and rendering is
      // pointless. Set flush_context_ to null as well, so if we get another
      // DidChangeView call, the main loop is started again.
      flush_context_ = context_;
      return;
    }

    Update();
    Render();
    // Store a reference to the context that is being flushed; this ensures
    // the callback is called, even if context_ changes before the flush
    // completes.
    flush_context_ = context_;
    context_.Flush(callback_factory_.NewCallback(&Instance::MainLoop));
    frames_drawn_++;
    UpdateFps();
  }

  void UpdateFps() {
    struct timeval current_frame_time;
    gettimeofday(&current_frame_time, NULL);

    int diff_ms = TimeDeltaMs(&last_frame_time_, &current_frame_time);
    if (diff_ms > kFpsUpdateMs) {
      real fps = static_cast<real>(frames_drawn_ * 1000) / diff_ms;
      PostMessage(fps);
      frames_drawn_ = 0;
      last_frame_time_ = current_frame_time;
    }
  }

  pp::CompletionCallbackFactory<Instance> callback_factory_;
  pp::Graphics2D context_;
  pp::Graphics2D flush_context_;
  pp::Size context_size_;

  SimulationConfig simulation_config_;
  Simulation simulation_;
  PaletteConfig palette_config_;
  Palette palette_;

  int scale_numer_;
  int scale_denom_;

  pp::Point mouse_point_;
  bool mouse_down_;
  real brush_radius_;
  real brush_color_;

  int frames_drawn_;
  struct timeval last_frame_time_;
};

class Module : public pp::Module {
 public:
  Module() : pp::Module() {}
  virtual ~Module() {}

  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new Instance(instance);
  }
};

namespace pp {
Module* CreateModule() { return new ::Module(); }
}  // namespace pp
