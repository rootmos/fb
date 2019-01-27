export PREFIX=$(shell readlink -f ./root)
CC=$(PREFIX)/bin/arm-linux-gnueabihf-gcc
LD=$(CC)
CFLAGS = -Wall -Werror -Iinclude -I$(PREFIX)/include
EXTRA_CFLAGS ?= -O2
EXTRA_LDFLAGS ?=
LIBS = -L$(PREFIX)/lib -lasound -l:libr.a
SRC=$(shell git ls-files)

FB=/dev/fb0
DEPLOY_HOST=pi@192.168.1.166
DEPLOY_TARGET=$(DEPLOY_HOST):demo
DEPLOY_LOCAL_EXECUTABLE=./demo

deploy: demo
	scp $< $(DEPLOY_TARGET)
	ssh $(DEPLOY_HOST) $(DEPLOY_LOCAL_EXECUTABLE) $(FB)

demo: demo.o fb.o mark.o
	$(LD) $(EXTRA_LDFLAGS) -o $@ $^ $(LIBS)

toolchain:
	$(MAKE) -C $@

libr:
	$(MAKE) -C $@ install CC=$(CC) LD=$(LD)

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c $<

clean:
	rm -rf *.o demo

.PHONY: run clean toolchain deploy libr
