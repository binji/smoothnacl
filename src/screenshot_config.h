// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SCREENSHOT_CONFIG_H_
#define SCREENSHOT_CONFIG_H_

#include <memory>
#include <vector>

class ImageOperation;

struct ScreenshotConfig {
  typedef std::shared_ptr<ImageOperation> OperationPtr;
  typedef std::vector<OperationPtr> Operations;
  Operations operations;
};

#endif  // SCREENSHOT_CONFIG_H_
