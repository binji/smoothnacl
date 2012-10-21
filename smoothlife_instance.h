// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXAMPLES_SMOOTHLIFE_SMOOTHLIFE_H_
#define EXAMPLES_SMOOTHLIFE_SMOOTHLIFE_H_

#include <deque>
#include "ppapi/cpp/instance.h"
#include "ppapi/utility/completion_callback_factory.h"
#include "fft_allocation.h"
#include "locked_object.h"
#include "task_queue.h"


class SmoothlifeThread;
class SmoothlifeView;


class SmoothlifeInstance : public pp::Instance {
 public:
  explicit SmoothlifeInstance(PP_Instance instance);
  virtual ~SmoothlifeInstance();
  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);
  virtual void DidChangeView(const pp::View& view);
  virtual bool HandleInputEvent(const pp::InputEvent& event);
  virtual void HandleMessage(const pp::Var& var_message);

 private:
  void EnqueueTask(Task* task);

  pp::CompletionCallbackFactory<SmoothlifeInstance> factory_;
  SmoothlifeView* view_;
  bool is_initial_view_change_;
  SmoothlifeThread* thread_;
  LockedObject<AlignedReals>* locked_buffer_;
  LockedObject<TaskQueue>* task_queue_;

  // Disallow copy constructor and assignment operator.
  SmoothlifeInstance(const SmoothlifeInstance&);
  SmoothlifeInstance& operator=(const SmoothlifeInstance&);
};

#endif  // EXAMPLES_SMOOTHLIFE_SMOOTHLIFE_H_
