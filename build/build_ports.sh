set -o errexit

if [ -z "${NACL_SDK_ROOT:-}" ]; then
  echo "-------------------------------------------------------------------"
  echo "NACL_SDK_ROOT is unset."
  echo "This environment variable needs to be pointed at some version of"
  echo "the Native Client SDK (the directory containing toolchain/)."
  echo "NOTE: set this to an absolute path."
  echo "-------------------------------------------------------------------"
  exit -1
fi

readonly SCRIPT_DIR=$(dirname $(readlink -f $0))
readonly ROOT_DIR=$(dirname ${SCRIPT_DIR})
readonly NACLPORTS_DIR=${SCRIPT_DIR}/naclports
readonly LIBS="fftw"

cd ${NACLPORTS_DIR}
make NACL_ARCH=i686 ${LIBS}
make NACL_ARCH=x86_64 ${LIBS}
make NACL_ARCH=arm ${LIBS}
make NACL_ARCH=pnacl ${LIBS}
