#!/bin/bash

# Builds smooth.js and smooth.data which is embedded by smooth.html.
# After building, serve this directory and browse to <localhost>/smooth.html

# To build:
# 1) git clone git://github.com/kripken/emscripten.git   (requires clang/LLVM)
#    NOTE: You may need to modify tools/shared.py to point to the correct llvm
#    binary names (e.g., llvm-link -> llvm-link-3.0).
# 2) cd emscripten
# 3) patch -p0 < <path/to/emscripten_float_luminance.patch>
# 4) ./emcc      (one time setup)
# 5) EMSCRIPTEN_ROOT=<path/to/emscripten> ./build_emscripten.sh

set -e

if [[ ! ${EMSCRIPTEN_ROOT} || ! ${EMSCRIPTEN_ROOT-_} ]]; then
  echo "Set EMSCRIPTEN_ROOT to the location of your emscripten install."
  exit 1
fi

${EMSCRIPTEN_ROOT}/em++ hello_world.cc matrix.cc smoothlife.cc -o smooth.js \
  -s EXPORTED_FUNCTIONS="['_main']" \
  --preload-file 2D/copybuffercr.frag \
  --preload-file 2D/copybuffercr.vert \
  --preload-file 2D/draw.frag \
  --preload-file 2D/draw.vert \
  --preload-file 2D/fft.frag \
  --preload-file 2D/fft.vert \
  --preload-file 2D/kernelmul.frag \
  --preload-file 2D/kernelmul.vert \
  --preload-file 2D/snm.frag \
  --preload-file 2D/snm.vert \
  --preload-file 2D/copybufferrc.frag \
  --preload-file 2D/copybufferrc.vert

echo "Success!"
