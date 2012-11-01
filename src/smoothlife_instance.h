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
#include "condvar.h"
#include "fft_allocation.h"
#include "locked_object.h"
#include "task_queue.h"


class SmoothlifeInstance;
typedef std::vector<std::string> ParamList;
typedef void (SmoothlifeInstance::*MessageFunc)(const ParamList&);
typedef std::map<std::string, MessageFunc> MessageMap;

namespace cpu {
class SmoothlifeThread;
class SmoothlifeView;
class ThreadContext;
}


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
                         cpu::ThreadContext* context);

  void InitMessageMap();
  void MessageSetKernel(const ParamList& params);
  void MessageSetSmoother(const ParamList& params);
  void MessageClear(const ParamList& params);
  void MessageSplat(const ParamList& params);
  void MessageSetRunOptions(const ParamList& params);
  void MessageSetDrawOptions(const ParamList& params);
  void MessageSetFullscreen(const ParamList& params);

  void EnqueueTask(Task* task);
  void ScheduleUpdate();
  void UpdateCallback(int32_t result);

  pp::CompletionCallbackFactory<SmoothlifeInstance> factory_;
  cpu::SmoothlifeView* view_;
  cpu::SmoothlifeThread* thread_;
  pp::Size sim_size_;
  LockedObject<AlignedReals>* locked_buffer_;
  LockedObject<TaskQueue>* task_queue_;
  LockedObject<int>* frames_drawn_;
  CondVar* step_cond_;
  MessageMap message_map_;
  pp::Fullscreen fullscreen_;
  bool is_initial_view_change_;

  // Disallow copy constructor and assignment operator.
  SmoothlifeInstance(const SmoothlifeInstance&);
  SmoothlifeInstance& operator=(const SmoothlifeInstance&);
};

#endif  // EXAMPLES_SMOOTHLIFE_SMOOTHLIFE_H_
