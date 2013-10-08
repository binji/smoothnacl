DATA_DIR?=out/chromium-data-dir
CHROME_PATH?=/home/binji/dev/chromium/src/out/Release/chrome
CHROME_ARGS?=--user-data-dir=${DATA_DIR} --enable-nacl ${CHROME_EXTRA_ARGS}

OUT_DIR=out
BUILD_NINJA=build.ninja
NACL_SDK_ROOT=${OUT_DIR}/nacl_sdk/pepper_29
NINJA=${OUT_DIR}/ninja
NINJA_WRAP=build/ninja-wrap/ninja_wrap.py

all: ${BUILD_NINJA} ${NINJA} ${NACL_SDK_ROOT}
	@${NINJA}

${OUT_DIR}:
	@mkdir -p ${OUT_DIR}

${NINJA}: | ${OUT_DIR}
	@echo "installing ninja"
	@cd build/third_party/ninja && ./bootstrap.py
	@cp build/third_party/ninja/ninja ${NINJA}

${BUILD_NINJA}: build/build.nw ${NINJA_WRAP}
	@python ${NINJA_WRAP} $< -o $@ -D nacl_sdk_root=${NACL_SDK_ROOT}

.PHONY: ports
ports: | ${NACL_SDK_ROOT}
	@./build/build_ports.sh

clean:
	@rm -rf ${OUT_DIR} ${BUILD_NINJA}

runclean: all
	@rm -rf ${DATA_DIR}
	@${CHROME_PATH} ${NEXE_ARGS}

run: all
	@python ${NACL_SDK_ROOT}/tools/run.py -C ${PWD}/out/ -- ${CHROME_PATH} ${CHROME_ARGS}

.PHONY: all clean runclean run ports
