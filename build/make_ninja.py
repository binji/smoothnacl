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
  'common.js',
  'example.js',
])


def main():
  parser = optparse.OptionParser()
  options, args = parser.parse_args()

  out_filename = os.path.join(os.path.dirname(__file__), '../build.ninja')
  f = cStringIO.StringIO()
  w = ninja_syntax.Writer(f)

  w.rule('configure', command = MAKE_NINJA, generator=1)
  w.build('build.ninja', 'configure', implicit=[MAKE_NINJA])

  w.variable('nacl_sdk_usr', 'nacl_sdk/pepper_23')
  w.variable('toolchain_dir', '$nacl_sdk_usr/toolchain/linux_x86_newlib')

  Code(w)
  Data(w)

  # Don't write build.ninja until everything succeeds
  open(out_filename, 'w').write(f.getvalue())


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

  flags = '-g -std=c++0x -O2'

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
  for data in DATA_FILES:
    w.build(os.path.join('out', os.path.basename(data)), 'cp', data)


if __name__ == '__main__':
  sys.exit(main())
