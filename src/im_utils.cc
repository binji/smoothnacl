// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "im_utils.h"
#include <im_format_all.h>

imBinFileBase* iBinSystemFileNewFunc() {
  return NULL;
}

imBinFileBase* iBinSystemFileHandleNewFunc() {
  return NULL;
}

void imFormatRegisterInternal() {
  imBinFileSetCurrentModule(IM_MEMFILE);
  imFormatRegisterJPEG();
  imFormatRegisterPNG();
}
