// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TASK_QUEUE_H_
#define TASK_QUEUE_H_

#include <deque>

class Task;
typedef std::deque<Task*> TaskQueue;

#endif  // TASK_QUEUE_H_
