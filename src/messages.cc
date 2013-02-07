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

#include "messages.h"
#include <array>
#include <algorithm>
#include <ppapi/cpp/fullscreen.h>
#include <stdlib.h>
#include "image_operation.h"
#include "kernel_config.h"
#include "message_handler.h"
#include "palette.h"
#include "screenshot_config.h"
#include "simulation_thread.h"
#include "simulation_thread_options.h"
#include "smoother_config.h"

namespace msg {

void Clear(SimulationThread* thread, const ParamList& params) {
  if (params.size() != 1)
    return;

  double color = strtod(params[0].c_str(), NULL);
  thread->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskClear, color));
}

void Screenshot(SimulationThread* thread, const ParamList& params) {
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

  thread->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskScreenshot,
                                       config));
}

void SetBrush(const ParamList& params, double* out_radius, double* out_color) {
  const double kMaxBrushRadius = 100.0;

  if (params.size() < 2)
    return;

  *out_radius = strtod(params[0].c_str(), NULL);
  *out_color = strtod(params[1].c_str(), NULL);

  // Clamp values.
  *out_radius = std::max(std::min(*out_radius, kMaxBrushRadius), 0.0);
  *out_color = std::max(std::min(*out_color, 1.0), 0.0);
}

void SetDrawOptions(SimulationThread* thread, const ParamList& params) {
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

  thread->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskSetDrawOptions,
                                       draw_options));
}

void SetFullscreen(const ParamList& params, pp::Fullscreen* fullscreen) {
  if (params.size() != 1)
    return;

  fullscreen->SetFullscreen(params[0] == "true");
}

void SetKernel(SimulationThread* thread, const ParamList& params) {
  if (params.size() != 3)
    return;

  KernelConfig config;
  config.disc_radius = strtod(params[0].c_str(), NULL);
  config.ring_radius = strtod(params[1].c_str(), NULL);
  config.blend_radius = strtod(params[2].c_str(), NULL);
  thread->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskSetKernel,
                                       config));
}

void SetPalette(SimulationThread* thread, const ParamList& params) {
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
  thread->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskSetPalette,
                                       config));
}

void SetRunOptions(SimulationThread* thread, const ParamList& params) {
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

  thread->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskSetRunOptions,
                                       run_options));
  thread->Step();
}

void SetSmoother(SimulationThread* thread, const ParamList& params) {
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
  thread->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskSetSmoother,
                                       config));
}

void Splat(SimulationThread* thread, const ParamList& params) {
  if (params.size() != 0)
    return;

  thread->EnqueueTask(MakeFunctionTask(&SimulationThread::TaskSplat));
}

}  // namespace msg
