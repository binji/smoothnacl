DATA_DIR?=out/chromium-data-dir
CHROME_PATH?=/home/binji/dev/chromium/src/out/Release/chrome
NEXE_ARGS?=--enable-nacl --user-data-dir=${DATA_DIR}

all: build.ninja
	@ninja

build.ninja: build/make_ninja.py
	@python build/make_ninja.py

clean:
	@rm -rf out build.ninja

runclean: all
	@rm -rf ${DATA_DIR}
	@python script/run.py out ${CHROME_PATH} ${NEXE_ARGS}

run: all
	@python script/run.py out ${CHROME_PATH} ${NEXE_ARGS}

package: build.ninja
	@ninja package

.PHONY: all clean runclean run package
