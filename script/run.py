#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys
import test_server

if __name__ == '__main__':
  server = test_server.LocalHTTPServer(sys.argv[1])
  try:
    subprocess.call(sys.argv[2:] + [server.GetURL('')])
  finally:
    server.Shutdown()
