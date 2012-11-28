#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import cStringIO
import ninja_syntax
import optparse
import os
import sys

WINDOWS = sys.platform in ('cygwin', 'win32')
SCRIPT_DIR = os.path.dirname(__file__)
ROOT_DIR = os.path.dirname(SCRIPT_DIR)


def Prefix(prefix, items):
  return ' '.join(prefix + x for x in items.split())

def SourceToObj(source, bits):
  return os.path.join('out', '%s.%s.o' % (os.path.splitext(source)[0], bits))

def SplitPath(path):
  result = []
  while True:
    head, tail = os.path.split(path)
    if not head:
      return [tail] + result
    result[:0] = [tail]
    path = head

def NoRepath(seq):
  result = []
  for path in seq:
    result.append(path.replace('<', '').replace('>', ''))
  return result

def Repath(prefix, seq):
  result = []
  for path in seq:
    if '<' in path:
      strip_start = path.find('<')
      strip_end = path.find('>')
      path = path[:strip_start] + path[strip_end+1:]
    else:
      path = os.path.join(*SplitPath(path)[1:])

    if type(prefix) is list:
      args = prefix + [path]
      result.append(os.path.join(*args))
    else:
      result.append(os.path.join(prefix, path))
  return result

def Python(cmd):
  if WINDOWS:
    return 'python %s' % (cmd,)
  return cmd


def Path(p):
  return os.path.normpath(p)



SHADER_FILES = [
  'data/shaders/1tex.vert',
  'data/shaders/2tex.vert',
  'data/shaders/3tex.vert',
  'data/shaders/draw_circle.vert',
  'data/shaders/complex_real.frag',
  'data/shaders/real_complex.frag',
  'data/shaders/draw.frag',
  'data/shaders/draw_circle.frag',
  'data/shaders/fft.frag',
  'data/shaders/kernelmul.frag',
  'data/shaders/smoother.frag',
]
OUT_SHADER_CC = 'out/gen/shader_source.cc'
OUT_SHADER_H = 'out/gen/shader_source.h'

MAKE_NINJA = os.path.relpath(__file__, ROOT_DIR)
SOURCE_FILES = [
  'src/condvar.cc',
  'src/cpu/draw_strategy.cc',
  'src/cpu/initializer_factory.cc',
  'src/cpu/kernel.cc',
  'src/cpu/simulation.cc',
  'src/cpu/smoother.cc',
  'src/cpu/view.cc',
  'src/functions.cc',
  'src/gpu/complex_to_real.cc',
  'src/gpu/draw_circle.cc',
  'src/gpu/draw_strategy.cc',
  'src/gpu/fft.cc',
  'src/gpu/fft_stage.cc',
  'src/gpu/gl_task.cc',
  'src/gpu/initializer_factory.cc',
  'src/gpu/kernel.cc',
  'src/gpu/kernel_mul.cc',
  'src/gpu/locked_queue.cc',
  'src/gpu/real_to_complex.cc',
  'src/gpu/shader.cc',
  'src/gpu/simulation.cc',
  'src/gpu/smoother.cc',
  'src/gpu/task_functions.cc',
  'src/gpu/texture.cc',
  'src/gpu/vertex_buffer.cc',
  'src/gpu/view.cc',
  'src/gpu/wrap_gl.cc',
  'src/palette.cc',
  'src/smoothlife_instance.cc',
  'src/smoothlife_module.cc',
  'src/thread.cc',
  'src/view_base.cc',
  OUT_SHADER_CC,
]


DATA_FILES = [
  'data/index.html',
  'data/main.css',
  'data/main.js',
  # Strip everything between < and > when repathing.
  #'<third_party/Iris/iris.min.js',
  '<third_party/jquery/>jquery-1.8.2.min.js',
  '<third_party/jquery.layout/>jquery.layout-latest.min.js',
  '<third_party/jquery-miniColors/>images/colors.png',
  '<third_party/jquery-miniColors/>images/trigger.png',
  '<third_party/jquery-miniColors/>jquery.miniColors.css',
  '<third_party/jquery-miniColors/>jquery.miniColors.min.js',
  '<third_party/jquery-ui/>images/ui-bg_flat_0_aaaaaa_40x100.png',
  '<third_party/jquery-ui/>images/ui-bg_flat_75_ffffff_40x100.png',
  '<third_party/jquery-ui/>images/ui-bg_glass_55_fbf9ee_1x400.png',
  '<third_party/jquery-ui/>images/ui-bg_glass_65_ffffff_1x400.png',
  '<third_party/jquery-ui/>images/ui-bg_glass_75_dadada_1x400.png',
  '<third_party/jquery-ui/>images/ui-bg_glass_75_e6e6e6_1x400.png',
  '<third_party/jquery-ui/>images/ui-bg_glass_95_fef1ec_1x400.png',
  '<third_party/jquery-ui/>images/ui-bg_highlight-soft_75_cccccc_1x100.png',
  '<third_party/jquery-ui/>images/ui-icons_222222_256x240.png',
  '<third_party/jquery-ui/>images/ui-icons_2e83ff_256x240.png',
  '<third_party/jquery-ui/>images/ui-icons_454545_256x240.png',
  '<third_party/jquery-ui/>images/ui-icons_888888_256x240.png',
  '<third_party/jquery-ui/>images/ui-icons_cd0a0a_256x240.png',
  '<third_party/jquery-ui/>jquery-ui-1.9.0.custom.min.css',
  '<third_party/jquery-ui/>jquery-ui-1.9.0.custom.min.js',
  '<third_party/Nunito/>Nunito-Bold.ttf',
  '<third_party/Nunito/>Nunito-Regular.ttf',
]
SRC_DATA_FILES = NoRepath(DATA_FILES)
DST_DATA_FILES = Repath('out', DATA_FILES)


BUILT_FILES = [
  'out/smoothlife.nmf',
  'out/smoothlife_32.nexe',
  'out/smoothlife_64.nexe',
]


PACKAGE_FILES = DATA_FILES + BUILT_FILES + [
  'data/icon16.png',
  'data/icon64.png',
  'data/icon128.png',
  'data/background.js',
  'data/manifest.json',
]
SRC_PACKAGE_FILES = NoRepath(PACKAGE_FILES)
DST_PACKAGE_FILES = Repath(['out', 'package'], PACKAGE_FILES)


class Writer(ninja_syntax.Writer):
  def __init__(self, s):
    ninja_syntax.Writer.__init__(self, s)

  def build(self, outputs, rule, inputs=None, implicit=None, order_only=None,
            variables=None):
    outputs = map(Path, self._as_list(outputs))
    inputs = map(Path, self._as_list(inputs))
    implicit = map(Path, self._as_list(implicit))
    order_only = map(Path, self._as_list(order_only))
    ninja_syntax.Writer.build(self, outputs, rule, inputs, implicit, order_only,
                              variables)

def main():
  parser = optparse.OptionParser()
  options, args = parser.parse_args()

  out_filename = os.path.join(os.path.dirname(__file__), '../build.ninja')
  s = cStringIO.StringIO()
  w = Writer(s)

  w.rule('configure', command = Python(MAKE_NINJA), generator=1)
  w.build('build.ninja', 'configure', implicit=[MAKE_NINJA])

  platform_dict = {
    'linux2': 'linux',
    'cygwin': 'win',
    'win32': 'win',
    'darwin': 'mac'
  }

  w.variable('nacl_sdk_usr', Path('nacl_sdk/pepper_23'))
  w.variable('toolchain_dir', Path('$nacl_sdk_usr/toolchain/%s_x86_newlib' % (
      platform_dict[sys.platform])))

  Gen(w)
  Code(w)
  Data(w)
  Package(w)
  w.default(' '.join(map(Path, ['out/smoothlife.nmf'] + DST_DATA_FILES)))

  # Don't write build.ninja until everything succeeds
  with open(out_filename, 'w') as f:
    f.write(s.getvalue())


def Gen(w):
  w.newline()
  w.rule('shader_to_c',
      command=Python('script/shader_to_c.py -r out -o $outbase $in'),
      description='SHADER_TO_C $out')
  w.build([OUT_SHADER_CC, OUT_SHADER_H], 'shader_to_c', SHADER_FILES,
      variables={'outbase': os.path.splitext(OUT_SHADER_CC)[0]})


def Code(w):
  w.newline()
  w.rule('cc',
      command='$cc $cflags -MMD -MF $out.d -c $in -o $out',
      depfile='$out.d',
      description='CC $out')
  w.rule('link',
      command='$cc $in $ldflags -o $out',
      description='LINK $out')

  libs = Prefix('-l', '''pthread ppapi_gles2 ppapi_cpp ppapi fftw3''')

  flags = '-g -std=c++0x -O2 -msse2'
  fftw_dir = 'third_party/fftw-prebuilt'

  for bits, flavor in (('32', 'i686-nacl'), ('64', 'x86_64-nacl')):
    includes = '-Isrc -Iout '
    includes += '-I{fftw_dir}/newlib_x86_{bits}/include'.format(**vars())
    libdirs = '-L{fftw_dir}/newlib_x86_{bits}/lib'.format(**vars())

    w.variable('cflags' + bits, '{flags} {includes}'.format(**vars()))
    w.variable('ldflags' + bits, '{libdirs} {libs}'.format(**vars()))
    w.variable('cc' + bits,
               Path('$toolchain_dir/bin/{flavor}-g++'.format(**vars())))

    sources = SOURCE_FILES
    objs = [SourceToObj(x, bits) for x in sources]
    for source, obj in zip(sources, objs):
      w.build(obj, 'cc', source,
          order_only=OUT_SHADER_H,
          variables={'cflags': '$cflags' + bits, 'cc': '$cc' + bits})

    w.build('out/smoothlife_{bits}.nexe'.format(**vars()), 'link', objs,
        variables={'ldflags': '$ldflags' + bits,
                   'cc': '$cc' + bits})

  w.newline()
  w.rule('nmf',
      command='$nmf $in -o $out -t newlib -D$objdump',
      description='NMF $out')
  w.variable('nmf', Python('$nacl_sdk_usr/tools/create_nmf.py'))
  w.build('out/smoothlife.nmf', 'nmf',
      ['out/smoothlife_32.nexe', 'out/smoothlife_64.nexe'],
      variables={'objdump': '$toolchain_dir/bin/x86_64-nacl-objdump'})


def Data(w):
  w.newline()

  if WINDOWS:
    cmd = Python('script/cp.py $in $out')
  else:
    cmd = 'cp $in $out'
  w.rule('cp', command=cmd, description='CP $out')
  for outf, inf in zip(DST_DATA_FILES, SRC_DATA_FILES):
    w.build(outf, 'cp', inf)


def Package(w):
  w.newline()
  w.rule('zip', command='$zip -C out/package $out $in', description='ZIP $out')
  w.variable('zip', Python('script/zip.py'))
  for outf, inf in zip(DST_PACKAGE_FILES, SRC_PACKAGE_FILES):
    w.build(outf, 'cp', inf)
  w.build(os.path.join('out', 'smoothlife.zip'), 'zip', DST_PACKAGE_FILES)
  w.build('package', 'phony', 'out/smoothlife.zip')


if __name__ == '__main__':
  sys.exit(main())
