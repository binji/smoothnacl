#!/usr/bin/env python
import cStringIO
import ninja_syntax
import optparse
import os
import sys

SCRIPT_DIR = os.path.dirname(__file__)
ROOT_DIR = os.path.dirname(SCRIPT_DIR)


def PrefixPath(path, seq):
  return [os.path.join(path, x) for x in seq]

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

def Repath(prefix, seq):
  result = []
  for path in seq:
    path = os.path.join(*SplitPath(path)[1:])
    if type(prefix) is list:
      args = prefix + [path]
      result.append(os.path.join(*args))
    else:
      result.append(os.path.join(prefix, path))
  return result


MAKE_NINJA = os.path.relpath(__file__, ROOT_DIR)
SOURCE_FILES = PrefixPath('src', [
  'functions.cc',
  'kernel.cc',
  'simulation.cc',
  'smoother.cc',
  'smoothlife_instance.cc',
  'smoothlife_module.cc',
  'smoothlife_thread.cc',
  'smoothlife_view.cc',
])

DATA_FILES = PrefixPath('data', [
  'index.html',
  'jquery-1.8.2.min.js',
  'jquery.layout-latest.min.js',
  'jquery-ui-1.9.0.custom.min.css',
  'jquery-ui-1.9.0.custom.min.js',
  'main.css',
  'main.js',
  'images/ui-bg_flat_0_aaaaaa_40x100.png',
  'images/ui-bg_flat_75_ffffff_40x100.png',
  'images/ui-bg_glass_55_fbf9ee_1x400.png',
  'images/ui-bg_glass_65_ffffff_1x400.png',
  'images/ui-bg_glass_75_dadada_1x400.png',
  'images/ui-bg_glass_75_e6e6e6_1x400.png',
  'images/ui-bg_glass_95_fef1ec_1x400.png',
  'images/ui-bg_highlight-soft_75_cccccc_1x100.png',
  'images/ui-icons_222222_256x240.png',
  'images/ui-icons_2e83ff_256x240.png',
  'images/ui-icons_454545_256x240.png',
  'images/ui-icons_888888_256x240.png',
  'images/ui-icons_cd0a0a_256x240.png'
])


BUILT_FILES = PrefixPath('out', [
  'smoothlife.nmf',
  'smoothlife_32.nexe',
  'smoothlife_64.nexe',
])

PACKAGE_FILES = DATA_FILES + BUILT_FILES + PrefixPath('data', [
  'icon16.png',
  'icon64.png',
  'icon128.png',
  'background.js',
  'manifest.json',
])

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

  Code(w)
  Data(w)
  Package(w)
  w.default('out/smoothlife.nmf ' + ' '.join(Repath('out', DATA_FILES)))

  # Don't write build.ninja until everything succeeds
  with open(out_filename, 'w') as f:
    f.write(s.getvalue())


def Code(w):
  w.newline()
  w.rule('cc',
      command='$cc $cflags -MMD -MF $out.d -c $in -o $out',
      depfile='$out.d',
      description='CC $out')
  w.rule('link',
      command='$cc $in $ldflags -o $out',
      description='LINK $out')

  libs = Prefix('-l', '''pthread ppapi_cpp ppapi fftw3''')

  flags = '-g -std=c++0x -O2 -msse2'

  for bits, flavor in (('32', 'i686-nacl'), ('64', 'x86_64-nacl')):
    includes = '-Ilib/newlib_x86_{bits}/include'.format(**vars())
    libdirs = '-Llib/newlib_x86_{bits}/lib'.format(**vars())

    w.variable('cflags' + bits, '{flags} {includes}'.format(**vars()))
    w.variable('ldflags' + bits, '{libdirs} {libs}'.format(**vars()))
    w.variable('cc' + bits, '$toolchain_dir/bin/{flavor}-g++'.format(**vars()))

    sources = SOURCE_FILES
    objs = [SourceToObj(x, bits) for x in sources]
    for source, obj in zip(sources, objs):
      w.build(obj, 'cc', source,
          variables={'cflags': '$cflags' + bits, 'cc': '$cc' + bits})

    w.build('out/smoothlife_{bits}.nexe'.format(**vars()), 'link', objs,
        variables={'ldflags': '$ldflags' + bits,
                   'cc': '$cc' + bits})

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
  dest = Repath('out', DATA_FILES)
  for inf, outf in zip(dest, DATA_FILES):
    w.build(inf, 'cp', outf)


def Package(w):
  w.newline()
  w.rule('zip', command='$zip -C out/package $out $in', description='ZIP $out')
  w.variable('zip', 'script/zip.py')
  dest = Repath(['out', 'package'], PACKAGE_FILES)
  for inf, outf in zip(dest, PACKAGE_FILES):
    w.build(inf, 'cp', outf)
  w.build(os.path.join('out', 'smoothlife.zip'), 'zip', dest)
  w.build('package', 'phony', 'out/smoothlife.zip')


if __name__ == '__main__':
  sys.exit(main())
