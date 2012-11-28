CHROME_PATH?=/home/binji/dev/chromium/src/out/Debug/chrome
NEXE_ARGS?=--enable-nacl --incognito

all: build.ninja
	ninja

build.ninja:
	python build/make_ninja.py

clean:
	rm -rf out build.ninja

run: all
	python script/run.py out ${CHROME_PATH} ${NEXE_ARGS}

package: build.ninja
	ninja package

.PHONY: all clean run package
