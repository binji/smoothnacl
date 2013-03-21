DATA_DIR?=out/chromium-data-dir
CHROME_PATH?=/home/binji/dev/chromium/src/out/Release/chrome
NEXE_ARGS?=--load-extension=${CURDIR}/out/package --user-data-dir=${DATA_DIR}
NACL_SDK_ROOT?=nacl_sdk/pepper_canary
NINJA_WRAP=build/ninja-wrap/ninja_wrap.py

all: build.ninja
	@ninja

build.ninja: build/build.nw ${NINJA_WRAP}
	@python ${NINJA_WRAP} $< -o $@ -D nacl_sdk_root=${NACL_SDK_ROOT}

clean:
	@rm -rf out build.ninja

runclean: all
	@rm -rf ${DATA_DIR}
	@${CHROME_PATH} ${NEXE_ARGS}

run: all
	@${CHROME_PATH} ${NEXE_ARGS}

.PHONY: all clean runclean run
