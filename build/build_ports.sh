set -o errexit

readonly SCRIPT_DIR=$(dirname $(readlink -f $0))
readonly ROOT_DIR=$(dirname ${SCRIPT_DIR})
readonly OUT_DIR=${ROOT_DIR}/out
readonly NACL_SDK_ROOT=${OUT_DIR}/nacl_sdk/pepper_29
readonly NACLPORTS_DIR=${SCRIPT_DIR}/naclports
readonly LIBS="fftw"

export NACL_SDK_ROOT
cd ${NACLPORTS_DIR}
make NACL_ARCH=i686 ${LIBS}
make NACL_ARCH=x86_64 ${LIBS}
make NACL_ARCH=arm ${LIBS}
