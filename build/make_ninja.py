#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import cStringIO
import ninja_syntax
import optparse
import os
import sys

SCRIPT_DIR = os.path.dirname(__file__)
ROOT_DIR = os.path.dirname(SCRIPT_DIR)


def Prefix(prefix, items):
  if items is None:
    return ''
  if type(items) is str:
    items = items.split()
  return ' '.join(prefix + x for x in items)

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

IM_SOURCE_FILES = [
  'third_party/im/src/old_imcolor.c',
  'third_party/im/src/old_imresize.c',
  'third_party/im/src/im_attrib.cpp',
  'third_party/im/src/im_bin.cpp',
  'third_party/im/src/im_binfile.cpp',
  'third_party/im/src/im_colorhsi.cpp',
  'third_party/im/src/im_colormode.cpp',
  'third_party/im/src/im_colorutil.cpp',
  'third_party/im/src/im_compress.cpp',
  'third_party/im/src/im_convertbitmap.cpp',
  'third_party/im/src/im_convertcolor.cpp',
  'third_party/im/src/im_convertopengl.cpp',
  'third_party/im/src/im_converttype.cpp',
  'third_party/im/src/im_counter.cpp',
  'third_party/im/src/im_datatype.cpp',
  'third_party/im/src/im_dib.cpp',
  'third_party/im/src/im_dibxbitmap.cpp',
  'third_party/im/src/im_file.cpp',
  'third_party/im/src/im_filebuffer.cpp',
  'third_party/im/src/im_fileraw.cpp',
  'third_party/im/src/im_format.cpp',
#  'third_party/im/src/im_format_all.cpp',
#  'third_party/im/src/im_format_jpeg.cpp',
#  'third_party/im/src/im_format_png.cpp',
  'third_party/im/src/im_image.cpp',
  'third_party/im/src/im_lib.cpp',
  'third_party/im/src/im_palette.cpp',
  'third_party/im/src/im_rgb2map.cpp',
  'third_party/im/src/im_str.cpp',
  'third_party/im/src/old_im.cpp',
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

def main():
  parser = optparse.OptionParser()
  options, args = parser.parse_args()

  out_filename = os.path.join(os.path.dirname(__file__), '../build.ninja')
  s = cStringIO.StringIO()
  w = ninja_syntax.Writer(s)

  w.rule('configure', command = MAKE_NINJA, generator=1)
  w.build('build.ninja', 'configure', implicit=[MAKE_NINJA])

  w.variable('nacl_sdk_usr', 'nacl_sdk/pepper_23')
  w.variable('toolchain_dir', '$nacl_sdk_usr/toolchain/linux_x86_newlib')

  Gen(w)
  Code(w)
  Data(w)
  Package(w)
  w.default('out/smoothlife.nmf ' + ' '.join(DST_DATA_FILES))

  # Don't write build.ninja until everything succeeds
  with open(out_filename, 'w') as f:
    f.write(s.getvalue())


def Gen(w):
  w.newline()
  w.rule('shader_to_c',
      command='script/shader_to_c.py -r out -o $outbase $in',
      description='SHADER_TO_C $out')
  w.build([OUT_SHADER_CC, OUT_SHADER_H], 'shader_to_c', SHADER_FILES,
      variables={'outbase': os.path.splitext(OUT_SHADER_CC)[0]})


def BuildProject(w, name, rule, sources,
                 includedirs=None, libdirs=None, libs=None, order_only=None):
  includedirs = includedirs or []
  libdirs = libdirs or []
  libs = libs or []

  for bits, flavor in (('32', 'i686-nacl'), ('64', 'x86_64-nacl')):
    bit_incdirs = Prefix('-I', [x.format(**vars()) for x in includedirs])
    bit_libdirs = Prefix('-L', [x.format(**vars()) for x in libdirs])
    bit_libs = Prefix('-l', [x.format(**vars()) for x in libs])

    cflags_name = 'cflags{bits}_{name}'.format(**vars())
    ldflags_name = 'ldflags{bits}_{name}'.format(**vars())
    w.variable(cflags_name, '$base_ccflags {bit_incdirs}'.format(**vars()))
    w.variable(ldflags_name, '{bit_libdirs} {bit_libs}'.format(**vars()))

    objs = [SourceToObj(x, bits) for x in sources]
    for source, obj in zip(sources, objs):
      w.build(obj, 'cc', source,
          order_only=order_only,
          variables={'cflags': '$' + cflags_name, 'cc': '$cc' + bits})

    if rule == 'link':
      out_name = 'out/{name}_{bits}.nexe'.format(**vars())
    elif rule == 'ar':
      out_name = 'out/{name}_{bits}.a'.format(**vars())

    w.build(out_name, rule, objs,
        variables={'ldflags': '$' + ldflags_name, 'cc': '$cc' + bits})


def Code(w):
  w.newline()
  w.rule('cc',
      command='$cc $cflags -MMD -MF $out.d -c $in -o $out',
      depfile='$out.d',
      description='CC $out')
  w.rule('ar',
      command='$ar rc $out $in',
      description='AR $out')
  w.rule('link',
      command='$cc $in $ldflags -o $out',
      description='LINK $out')

  w.variable('base_ccflags',  '-g -std=c++0x -O2 -msse2')
  for bits, flavor in (('32', 'i686-nacl'), ('64', 'x86_64-nacl')):
    w.variable('cc' + bits, '$toolchain_dir/bin/{flavor}-g++'.format(**vars()))
    w.variable('ar' + bits, '$toolchain_dir/bin/{flavor}-ar'.format(**vars()))

  fftw_dir = 'third_party/fftw-prebuilt'

  BuildProject(
    w, 'smoothlife', 'link',
    SOURCE_FILES,
    includedirs=[
      'src',
      'out',
      fftw_dir + '/newlib_x86_{bits}/include',
    ],
    libdirs=[fftw_dir + '/newlib_x86_{bits}/lib'],
    libs=['ppapi_gles2', 'ppapi_cpp', 'ppapi', 'fftw3'],
    order_only=OUT_SHADER_H)

  BuildProject(
    w, 'libim', 'ar',
    IM_SOURCE_FILES,
    includedirs=['third_party/im/include'])

  w.newline()
  w.rule('nmf',
      command='$nmf $in -o $out -t newlib -D$objdump',
      description='NMF $out')
  w.variable('nmf', '$nacl_sdk_usr/tools/create_nmf.py')
  w.build('out/smoothlife.nmf', 'nmf',
      ['out/smoothlife_32.nexe', 'out/smoothlife_64.nexe'],
      variables={'objdump': '$toolchain_dir/bin/x86_64-nacl-objdump'})


def Data(w):
  w.newline()
  w.rule('cp', command='cp $in $out', description='CP $out')
  for inf, outf in zip(DST_DATA_FILES, SRC_DATA_FILES):
    w.build(inf, 'cp', outf)


def Package(w):
  w.newline()
  w.rule('zip', command='$zip -C out/package $out $in', description='ZIP $out')
  w.variable('zip', 'script/zip.py')
  for inf, outf in zip(DST_PACKAGE_FILES, SRC_PACKAGE_FILES):
    w.build(inf, 'cp', outf)
  w.build(os.path.join('out', 'smoothlife.zip'), 'zip', DST_PACKAGE_FILES)
  w.build('package', 'phony', 'out/smoothlife.zip')


if __name__ == '__main__':
  sys.exit(main())
