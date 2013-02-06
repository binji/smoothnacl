#!/usr/bin/env python
# Copyright 2013 Ben Smith. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import optparse
import os
import sys


HEADER = \
"""// Copyright 2013 Ben Smith. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

"""


def FilenameToConst(filename):
  return 'shader_source_%s' % filename.replace('.', '_').lower()


def FilenameToGuard(filepath, rel_dir):
  filepath = os.path.relpath(filepath, rel_dir)
  return '%s_' % (filepath.replace('/', '_').replace('\\', '_')
      .replace('.', '_').upper(),)


def CRepr(line):
  r = repr(line)
  if r[0] == "'":
    return '"%s"' % (r[1:-1].replace('"', '\\"'))
  return r


def WriteHHeader(hfile, path, rel_dir):
  hfile.write(HEADER)
  guard = FilenameToGuard(path, rel_dir)
  hfile.write('#ifndef %s\n' % guard)
  hfile.write('#define %s\n\n' % guard)


def WriteHFooter(hfile, path, rel_dir):
  hfile.write('\n#endif  // %s\n' % FilenameToGuard(path, rel_dir))


def WriteCCHeader(ccfile, path, rel_dir):
  ccfile.write(HEADER)
  ccfile.write('#include "%s"\n\n' % os.path.relpath(path, rel_dir))


def main(args):
  parser = optparse.OptionParser()
  parser.add_option('-o', dest='out',
      help='base output name. (will generate .cc and .h)')
  parser.add_option('-r', dest='relative', default='.',
      help='generate path names relative to this directory.')
  options, args = parser.parse_args(args)
  if not options.out:
    parser.error('Must use -o to specify output file.')

  header = options.out + '.h'
  cc = options.out + '.cc'

  hfile = open(header, 'w')
  with open(header, 'w') as hfile:
    WriteHHeader(hfile, header, options.relative)
    for file_path in args:
      filename = os.path.basename(file_path)
      hfile.write('extern const char* %s;\n' % FilenameToConst(filename))
    WriteHFooter(hfile, header, options.relative)

  with open(cc, 'w') as ccfile:
    WriteCCHeader(ccfile, header, options.relative)
    for file_path in args:
      filename = os.path.basename(file_path)
      ccfile.write('const char* %s = \n' % FilenameToConst(filename))
      with open(file_path) as f:
        for line in f.readlines():
          ccfile.write('  %s\n' % (CRepr(line),))
      ccfile.write(';\n\n');


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
