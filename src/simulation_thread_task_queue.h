// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SIMULATION_THREAD_TASK_QUEUE_H_
#define SIMULATION_THREAD_TASK_QUEUE_H_

#include <memory>
#include <vector>
#include "task.h"

class SimulationThread;
typedef Task<SimulationThread> SimulationThreadTask;
typedef std::vector<std::shared_ptr<SimulationThreadTask> >
    SimulationThreadTaskQueue;

#endif  // SIMULATION_THREAD_TASK_QUEUE_H_
