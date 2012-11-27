CHROME_PATH?=/home/binji/dev/chromium/src/out/Release/chrome
NEXE_ARGS?=--enable-nacl --incognito

all: build.ninja
	@ninja

build.ninja:
	@build/make_ninja.py

clean:
	rm -rf out build.ninja

run: all
	script/run.py out ${CHROME_PATH} ${NEXE_ARGS}

package: build.ninja
	@ninja package

.PHONY: all clean kill run package
