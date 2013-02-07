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

#ifndef INSTANCE_H_
#define INSTANCE_H_

#include <ppapi/cpp/fullscreen.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/utility/completion_callback_factory.h>
#include "message_handler.h"


class SimulationThread;
class SimulationThreadContext;
class ViewBase;


class Instance : public pp::Instance {
 public:
  explicit Instance(PP_Instance instance);
  virtual ~Instance();
  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);
  virtual void DidChangeView(const pp::View& view);
  virtual bool HandleInputEvent(const pp::InputEvent& event);
  virtual void HandleMessage(const pp::Var& var_message);

 private:
  void ParseInitMessages(uint32_t argc, const char* argn[], const char* argv[]);
  void InitMessageMap();

  void ScheduleUpdate();
  void UpdateCallback(int32_t result);

  pp::CompletionCallbackFactory<Instance> factory_;
  ViewBase* view_;
  SimulationThread* thread_;
  pp::Fullscreen fullscreen_;
  bool is_initial_view_change_;
  MessageHandler message_handler_;

  double brush_radius_;
  double brush_color_;

  // Disallow copy constructor and assignment operator.
  Instance(const Instance&);
  Instance& operator=(const Instance&);
};

#endif  // INSTANCE_H_
