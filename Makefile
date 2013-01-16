DATA_DIR?=out/chromium-data-dir
CHROME_PATH?=/home/binji/dev/chromium/src/out/Release/chrome
NEXE_ARGS?=--load-extension=${CURDIR}/out/package --user-data-dir=${DATA_DIR}

all: build.ninja
	@ninja package

build.ninja: build/make_ninja.py
	@python build/make_ninja.py

clean:
	@rm -rf out build.ninja

runclean: all
	@rm -rf ${DATA_DIR}
	@${CHROME_PATH} ${NEXE_ARGS}

run: all
	@${CHROME_PATH} ${NEXE_ARGS}

.PHONY: all clean runclean run
