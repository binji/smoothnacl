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

def Python(cmd):
  if WINDOWS:
    return 'python %s' % (cmd,)
  return cmd


def Path(p):
  return os.path.normpath(p)


def PathToLibname(p):
  basename = os.path.splitext(os.path.basename(p))[0]
  assert(basename.startswith('lib'))
  return basename[3:]


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
  'src/im_utils.cc',
  'src/image_operation.cc',
  'src/palette.cc',
  'src/screenshot_task.cc',
  'src/simulation_thread.cc',
  'src/smoothlife_instance.cc',
  'src/smoothlife_module.cc',
  'src/view_base.cc',
  'src/worker_thread.cc',
  OUT_SHADER_CC,
]

ZLIB_SOURCE_FILES = [
  'third_party/im/src/zlib/adler32.c',
  'third_party/im/src/zlib/compress.c',
  'third_party/im/src/zlib/crc32.c',
  'third_party/im/src/zlib/deflate.c',
  'third_party/im/src/zlib/gzclose.c',
  'third_party/im/src/zlib/gzlib.c',
  'third_party/im/src/zlib/gzread.c',
  'third_party/im/src/zlib/gzwrite.c',
  'third_party/im/src/zlib/infback.c',
  'third_party/im/src/zlib/inffast.c',
  'third_party/im/src/zlib/inflate.c',
  'third_party/im/src/zlib/inftrees.c',
  'third_party/im/src/zlib/trees.c',
  'third_party/im/src/zlib/uncompr.c',
  'third_party/im/src/zlib/zutil.c',
]

PNG_SOURCE_FILES = [
  'third_party/im/src/libpng/png.c',
  'third_party/im/src/libpng/pngerror.c',
  'third_party/im/src/libpng/pngget.c',
  'third_party/im/src/libpng/pngmem.c',
  'third_party/im/src/libpng/pngpread.c',
  'third_party/im/src/libpng/pngread.c',
  'third_party/im/src/libpng/pngrio.c',
  'third_party/im/src/libpng/pngrtran.c',
  'third_party/im/src/libpng/pngrutil.c',
  'third_party/im/src/libpng/pngset.c',
  'third_party/im/src/libpng/pngtrans.c',
  'third_party/im/src/libpng/pngwio.c',
  'third_party/im/src/libpng/pngwrite.c',
  'third_party/im/src/libpng/pngwtran.c',
  'third_party/im/src/libpng/pngwutil.c',
]

JPEG_SOURCE_FILES = [
  'third_party/im/src/libjpeg/jaricom.c',
  'third_party/im/src/libjpeg/jcapimin.c',
  'third_party/im/src/libjpeg/jcapistd.c',
  'third_party/im/src/libjpeg/jcarith.c',
  'third_party/im/src/libjpeg/jccoefct.c',
  'third_party/im/src/libjpeg/jccolor.c',
  'third_party/im/src/libjpeg/jcdctmgr.c',
  'third_party/im/src/libjpeg/jchuff.c',
  'third_party/im/src/libjpeg/jcinit.c',
  'third_party/im/src/libjpeg/jcmainct.c',
  'third_party/im/src/libjpeg/jcmarker.c',
  'third_party/im/src/libjpeg/jcmaster.c',
  'third_party/im/src/libjpeg/jcomapi.c',
  'third_party/im/src/libjpeg/jcparam.c',
  'third_party/im/src/libjpeg/jcprepct.c',
  'third_party/im/src/libjpeg/jcsample.c',
  'third_party/im/src/libjpeg/jctrans.c',
  'third_party/im/src/libjpeg/jdapimin.c',
  'third_party/im/src/libjpeg/jdapistd.c',
  'third_party/im/src/libjpeg/jdarith.c',
  'third_party/im/src/libjpeg/jdatadst.c',
  'third_party/im/src/libjpeg/jdatasrc.c',
  'third_party/im/src/libjpeg/jdcoefct.c',
  'third_party/im/src/libjpeg/jdcolor.c',
  'third_party/im/src/libjpeg/jddctmgr.c',
  'third_party/im/src/libjpeg/jdhuff.c',
  'third_party/im/src/libjpeg/jdinput.c',
  'third_party/im/src/libjpeg/jdmainct.c',
  'third_party/im/src/libjpeg/jdmarker.c',
  'third_party/im/src/libjpeg/jdmaster.c',
  'third_party/im/src/libjpeg/jdmerge.c',
  'third_party/im/src/libjpeg/jdpostct.c',
  'third_party/im/src/libjpeg/jdsample.c',
  'third_party/im/src/libjpeg/jdtrans.c',
  'third_party/im/src/libjpeg/jerror.c',
  'third_party/im/src/libjpeg/jfdctflt.c',
  'third_party/im/src/libjpeg/jfdctfst.c',
  'third_party/im/src/libjpeg/jfdctint.c',
  'third_party/im/src/libjpeg/jidctflt.c',
  'third_party/im/src/libjpeg/jidctfst.c',
  'third_party/im/src/libjpeg/jidctint.c',
  'third_party/im/src/libjpeg/jmemmgr.c',
  'third_party/im/src/libjpeg/jmemnobs.c',
  'third_party/im/src/libjpeg/jquant1.c',
  'third_party/im/src/libjpeg/jquant2.c',
  'third_party/im/src/libjpeg/jutils.c',
]

EXIF_SOURCE_FILES = [
  'third_party/im/src/libexif/exif-byte-order.c',
  'third_party/im/src/libexif/exif-content.c',
  'third_party/im/src/libexif/exif-data.c',
  'third_party/im/src/libexif/exif-entry.c',
  'third_party/im/src/libexif/exif-format.c',
  'third_party/im/src/libexif/exif-ifd.c',
  'third_party/im/src/libexif/exif-loader.c',
  'third_party/im/src/libexif/exif-log.c',
  'third_party/im/src/libexif/exif-mem.c',
  'third_party/im/src/libexif/exif-mnote-data.c',
  'third_party/im/src/libexif/exif-tag.c',
  'third_party/im/src/libexif/exif-utils.c',
  'third_party/im/src/libexif/canon/exif-mnote-data-canon.c',
  'third_party/im/src/libexif/canon/mnote-canon-entry.c',
  'third_party/im/src/libexif/canon/mnote-canon-tag.c',
  'third_party/im/src/libexif/fuji/exif-mnote-data-fuji.c',
  'third_party/im/src/libexif/fuji/mnote-fuji-entry.c',
  'third_party/im/src/libexif/fuji/mnote-fuji-tag.c',
  'third_party/im/src/libexif/olympus/exif-mnote-data-olympus.c',
  'third_party/im/src/libexif/olympus/mnote-olympus-entry.c',
  'third_party/im/src/libexif/olympus/mnote-olympus-tag.c',
  'third_party/im/src/libexif/pentax/exif-mnote-data-pentax.c',
  'third_party/im/src/libexif/pentax/mnote-pentax-entry.c',
  'third_party/im/src/libexif/pentax/mnote-pentax-tag.c',
]

IM_SOURCE_FILES = [
  'third_party/im/src/im_attrib.cpp',
  'third_party/im/src/im_bin.cpp',
  'third_party/im/src/im_binfile.cpp',
  'third_party/im/src/im_colorhsi.cpp',
  'third_party/im/src/im_colormode.cpp',
  'third_party/im/src/im_colorutil.cpp',
  'third_party/im/src/im_convertbitmap.cpp',
  'third_party/im/src/im_convertcolor.cpp',
  'third_party/im/src/im_convertopengl.cpp',
  'third_party/im/src/im_converttype.cpp',
  'third_party/im/src/im_counter.cpp',
  'third_party/im/src/im_datatype.cpp',
  'third_party/im/src/im_filebuffer.cpp',
  'third_party/im/src/im_file.cpp',
  'third_party/im/src/im_fileraw.cpp',
  'third_party/im/src/im_format.cpp',
  'third_party/im/src/im_format_jpeg.cpp',
  'third_party/im/src/im_format_png.cpp',
  'third_party/im/src/im_image.cpp',
  'third_party/im/src/im_lib.cpp',
  'third_party/im/src/im_palette.cpp',
  'third_party/im/src/im_rgb2map.cpp',
  'third_party/im/src/im_str.cpp',
  'third_party/im/src/old_imcolor.c',
  'third_party/im/src/old_im.cpp',
  'third_party/im/src/old_imresize.c',
  'third_party/im/src/process/im_analyze.cpp',
  'third_party/im/src/process/im_arithmetic_bin.cpp',
  'third_party/im/src/process/im_arithmetic_un.cpp',
  'third_party/im/src/process/im_canny.cpp',
  'third_party/im/src/process/im_color.cpp',
  'third_party/im/src/process/im_convolve.cpp',
  'third_party/im/src/process/im_convolve_rank.cpp',
  'third_party/im/src/process/im_distance.cpp',
  'third_party/im/src/process/im_effects.cpp',
  'third_party/im/src/process/im_geometric.cpp',
  'third_party/im/src/process/im_histogram.cpp',
  'third_party/im/src/process/im_houghline.cpp',
  'third_party/im/src/process/im_kernel.cpp',
  'third_party/im/src/process/im_logic.cpp',
  'third_party/im/src/process/im_morphology_bin.cpp',
  'third_party/im/src/process/im_morphology_gray.cpp',
  'third_party/im/src/process/im_point.cpp',
  'third_party/im/src/process/im_process_counter.cpp',
  'third_party/im/src/process/im_quantize.cpp',
  'third_party/im/src/process/im_remotesens.cpp',
  'third_party/im/src/process/im_render.cpp',
  'third_party/im/src/process/im_resize.cpp',
  'third_party/im/src/process/im_statistics.cpp',
  'third_party/im/src/process/im_threshold.cpp',
  'third_party/im/src/process/im_tonegamut.cpp',
]


DATA_FILES = [
  'data/app.js',
  'data/controllers.js',
  'data/directives.js',
  'data/index.html',
  'data/main.css',
  'data/preset0.jpg',
  'data/preset1.jpg',
  'data/preset2.jpg',
  'data/preset3.jpg',
  'data/preset4.jpg',
  'data/preset5.jpg',
  'data/preset6.jpg',
  'data/preset7.jpg',
  'data/preset8.jpg',
  # Strip everything between < and > when repathing.
#  '<third_party/angular.js/>angular.min.js',
  '<third_party/angular.js/>angular.js',
  '<third_party/jquery/>jquery-1.8.2.min.js',
  '<third_party/jquery.layout/>jquery.layout-latest.min.js',
  '<third_party/jquery.masonry/>jquery.masonry.min.js',
  '<third_party/jquery-miniColors/>images/colors.png',
  '<third_party/jquery-miniColors/>images/trigger.png',
  '<third_party/jquery-miniColors/>jquery.miniColors.css',
#  '<third_party/jquery-miniColors/>jquery.miniColors.min.js',
  '<third_party/jquery-miniColors/>jquery.miniColors.js',
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
  '<third_party/jquery-ui/jquery-ui-1.9.0.custom/js/>jquery-ui-1.9.0.custom.js',
#  '<third_party/jquery-ui/>jquery-ui-1.9.0.custom.min.js',
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


def BuildProject(w, name, rule, sources,
                 includedirs=None, libs=None, order_only=None, defines=None):
  includedirs = includedirs or []
  libs = libs or []
  defines = defines or []
  libfiles = [l for l in libs if os.path.dirname(l)]
  libnames = [l for l in libs if not os.path.dirname(l)]
  libdirs = sorted(set([os.path.dirname(l) for l in libfiles]))
  libs = [PathToLibname(l) for l in libfiles] + libnames

  for bits, flavor in (('32', 'i686-nacl'), ('64', 'x86_64-nacl')):
    bit_incdirs = Prefix('-I', [x.format(**vars()) for x in includedirs])
    bit_libdirs = Prefix('-L', [x.format(**vars()) for x in libdirs])
    bit_libs = Prefix('-l', [x.format(**vars()) for x in libs])
    bit_libfiles = [x.format(**vars()) for x in libfiles]
    bit_defines = Prefix('-D', [x.format(**vars()) for x in defines])

    ccflags_name = 'ccflags{bits}_{name}'.format(**vars())
    cxxflags_name = 'cxxflags{bits}_{name}'.format(**vars())
    ldflags_name = 'ldflags{bits}_{name}'.format(**vars())
    w.variable(ccflags_name,
               '$base_ccflags {bit_incdirs} {bit_defines}'.format(**vars()))
    w.variable(cxxflags_name,
               '$base_cxxflags {bit_incdirs} {bit_defines}'.format(**vars()))
    w.variable(ldflags_name, '{bit_libdirs} {bit_libs}'.format(**vars()))

    objs = [SourceToObj(x, bits) for x in sources]
    for source, obj in zip(sources, objs):
      ext = os.path.splitext(source)[1]
      if ext in ('.cc', '.cpp'):
        cc = '$cxx' + bits
        ccflags = '$' + cxxflags_name
      elif ext == '.c':
        cc = '$cc' + bits
        ccflags = '$' + ccflags_name

      w.build(obj, 'cc', source,
              order_only=order_only,
              variables={'ccflags': ccflags, 'cc': cc})

    if rule == 'link':
      out_name = 'out/{name}_{bits}.nexe'.format(**vars())
      variables= {'ldflags': '$' + ldflags_name, 'cc': '$cxx' + bits}
    elif rule == 'ar':
      out_name = 'out/{name}_{bits}.a'.format(**vars())
      variables= {'ar': '$ar' + bits}

    w.build(out_name, rule, objs + bit_libfiles, variables=variables)


def Code(w):
  w.newline()
  w.rule('cc',
      command='$cc $ccflags -MMD -MF $out.d -c $in -o $out',
      depfile='$out.d',
      description='CC $out')
  w.rule('ar',
      command='$ar rc $out $in',
      description='AR $out')
  w.rule('link',
      command='$cc $in $ldflags -o $out',
      description='LINK $out')

#  w.variable('base_ccflags', '-g -msse2')
#  w.variable('base_cxxflags', '-g -std=c++0x -msse2')
  w.variable('base_ccflags', '-g -O2 -msse2')
  w.variable('base_cxxflags', '-g -std=c++0x -O2 -msse2')
  for bits, flavor in (('32', 'i686-nacl'), ('64', 'x86_64-nacl')):
    w.variable('cc' + bits,
               Path('$toolchain_dir/bin/{flavor}-gcc'.format(**vars())))
    w.variable('cxx' + bits,
               Path('$toolchain_dir/bin/{flavor}-g++'.format(**vars())))
    w.variable('ar' + bits,
               Path('$toolchain_dir/bin/{flavor}-ar'.format(**vars())))

  BuildProject(
    w, 'libim', 'ar',
    IM_SOURCE_FILES,
    includedirs=[
      'third_party/im/include',
      'third_party/im/src',
      'third_party/im/src/libexif',
      'third_party/im/src/libpng',
      'third_party/im/src/libjpeg',
    ],
    defines=['USE_EXIF'])

  BuildProject(
    w, 'libjpeg', 'ar',
    JPEG_SOURCE_FILES,
    includedirs=['third_party/im/include'])

  BuildProject(
    w, 'libexif', 'ar',
    EXIF_SOURCE_FILES,
    includedirs=[
      'third_party/im/src',
      'third_party/im/src/libexif',
    ])

  BuildProject(
    w, 'libpng', 'ar',
    PNG_SOURCE_FILES,
    includedirs=['third_party/im/src/zlib'])

  BuildProject(
    w, 'libzlib', 'ar',
    ZLIB_SOURCE_FILES)

  BuildProject(
    w, 'smoothlife', 'link',
    SOURCE_FILES,
    includedirs=[
      'src',
      'out',
      'third_party/fftw-prebuilt/newlib_x86_{bits}/include',
      'third_party/im/include'],
    libs=[
      'ppapi_gles2',
      'ppapi_cpp',
      'ppapi',
      'third_party/fftw-prebuilt/newlib_x86_{bits}/lib/libfftw3.a',
      'out/libim_{bits}.a',
      'out/libjpeg_{bits}.a',
      'out/libpng_{bits}.a',
      'out/libzlib_{bits}.a',
      'out/libexif_{bits}.a'],
    order_only=OUT_SHADER_H)

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
