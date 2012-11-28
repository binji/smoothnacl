// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXAMPLES_SMOOTHLIFE_SMOOTHLIFE_H_
#define EXAMPLES_SMOOTHLIFE_SMOOTHLIFE_H_

#include <map>
#include <string>
#include <vector>
#include <ppapi/cpp/size.h>
#include <ppapi/cpp/fullscreen.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/utility/completion_callback_factory.h>
#include "fft_allocation.h"
#include "task.h"


class SimulationThread;
class SimulationThreadContext;
class SmoothlifeInstance;
class ViewBase;
typedef std::vector<std::string> ParamList;
typedef void (SmoothlifeInstance::*MessageFunc)(const ParamList&);
typedef std::map<std::string, MessageFunc> MessageMap;


class SmoothlifeInstance : public pp::Instance {
 public:
  explicit SmoothlifeInstance(PP_Instance instance);
  virtual ~SmoothlifeInstance();
  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);
  virtual void DidChangeView(const pp::View& view);
  virtual bool HandleInputEvent(const pp::InputEvent& event);
  virtual void HandleMessage(const pp::Var& var_message);

 private:
  void ParseInitMessages(uint32_t argc, const char* argn[], const char* argv[],
                         SimulationThreadContext* context);

  void InitMessageMap();
  void MessageSetKernel(const ParamList& params);
  void MessageSetSmoother(const ParamList& params);
  void MessageSetPalette(const ParamList& params);
  void MessageClear(const ParamList& params);
  void MessageSplat(const ParamList& params);
  void MessageSetRunOptions(const ParamList& params);
  void MessageSetDrawOptions(const ParamList& params);
  void MessageSetFullscreen(const ParamList& params);
  void MessageScreenshot(const ParamList& params);

  void ScheduleUpdate();
  void UpdateCallback(int32_t result);

  pp::CompletionCallbackFactory<SmoothlifeInstance> factory_;
  ViewBase* view_;
  SimulationThread* thread_;
  pp::Size sim_size_;
  MessageMap message_map_;
  pp::Fullscreen fullscreen_;
  bool is_initial_view_change_;

  // Disallow copy constructor and assignment operator.
  SmoothlifeInstance(const SmoothlifeInstance&);
  SmoothlifeInstance& operator=(const SmoothlifeInstance&);
};

#endif  // EXAMPLES_SMOOTHLIFE_SMOOTHLIFE_H_
