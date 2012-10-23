CHROME_PATH?=/home/binji/dev/chromium/src/out/Debug/chrome
NEXE_ARGS?=--enable-nacl --incognito

all: build.ninja
	@ninja

build.ninja:
	@build/make_ninja.py

clean:
	rm -rf out

run: all
	script/run.py out ${CHROME_PATH} ${NEXE_ARGS}

package:
	@ninja package

.PHONY: all clean kill run package
