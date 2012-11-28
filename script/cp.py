#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import shutil
import optparse
import sys


def main(args):
  parser = optparse.OptionParser(usage='%prog [in] [out]')
  options, args = parser.parse_args(args)

  if len(args) != 2:
    parser.error('Expected input and output path.')

  print args[0], args[1]
  shutil.copyfile(os.path.normpath(args[0]), os.path.normpath(args[1]))
  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
