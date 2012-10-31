#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import os
import sys
import zipfile

def main(args):
  parser = optparse.OptionParser()
  parser.add_option('-C', dest='chdir', default='.',
      help='Change to directory before zipping.')
  options, args = parser.parse_args(args)

  z = zipfile.ZipFile(args[0], 'w', compression=zipfile.ZIP_DEFLATED)
  os.chdir(options.chdir)

  try:
    for f in args[1:]:
      z.write(os.path.relpath(f, options.chdir))
  finally:
    z.close()

if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
