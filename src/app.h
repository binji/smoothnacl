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

#ifndef APP_H_
#define APP_H_

#include <ppapi/cpp/point.h>

#include "ppapi_simple/ps_context_2d.h"
#include "ppapi_simple/ps_event.h"

#include "palette.h"
#include "simulation.h"
#include "simulation_config.h"

namespace pp {
class InputEvent;
class Var;
}

class App {
 public:
  App();
  ~App();

  void Run();

 private:
  void HandleEvent(PSEvent* event);
  void HandleDidChangeView(PSEvent* event);
  pp::Point ScreenToSim(const pp::Point& p) const;
  void HandleInput(const pp::InputEvent& event);
  void HandleMessage(const pp::Var& var);
  void Render();
  void Update();

  PSContext2D_t* context_;
  SimulationConfig simulation_config_;
  Simulation simulation_;
  PaletteConfig palette_config_;
  Palette palette_;

  double screen_to_sim_scale_;
  double screen_to_sim_x_offset_;
  double screen_to_sim_y_offset_;

  pp::Point mouse_point_;
  bool mouse_down_;
  double brush_radius_;
  double brush_color_;
};

#endif  // APP_H_
