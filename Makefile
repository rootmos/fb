TOOLCHAIN=$(shell realpath toolchain/root)
CC=$(TOOLCHAIN)/bin/arm-linux-gnueabihf-gcc
LD=$(CC)
CFLAGS = -Wall -Werror -I$(TOOLCHAIN)/include
EXTRA_CFLAGS ?= -O2
LDFLAGS = -L$(TOOLCHAIN)/lib -lasound
EXTRA_LDFLAGS ?=
SRC=$(shell git ls-files)

FB=/dev/fb0
DEPLOY_HOST=pi@192.168.1.167
DEPLOY_TARGET=$(DEPLOY_HOST):demo
DEPLOY_LOCAL_EXECUTABLE=./demo

deploy: demo
	scp $< $(DEPLOY_TARGET)
	ssh $(DEPLOY_HOST) $(DEPLOY_LOCAL_EXECUTABLE) $(FB)

run: demo
	./$< > /dev/fb0

demo: demo.o util.o
	$(LD) $(LDFLAGS) -o $@ $^

toolchain:
	$(MAKE) -C $@

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c $<

clean:
	rm -rf *.o demo

.PHONY: run clean toolchain deploy
