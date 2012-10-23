CHROME_PATH?=/home/binji/dev/chromium/src/out/Debug/chrome
NEXE_ARGS?=--enable-nacl --incognito

all: build.ninja
	@ninja

build.ninja:
	@./build/make_ninja.py

clean:
	rm -rf out

httpd.pid:
	mkdir out
	./httpd.py out 2>&1 > /dev/null &

kill:
	kill `cat httpd.pid` && rm httpd.pid

run: httpd.pid all
	${CHROME_PATH} ${NEXE_ARGS} http://localhost:5103

.PHONY: all kill
