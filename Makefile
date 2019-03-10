BUILD=$(shell readlink -f ./build)
TOOLCHAIN=$(shell readlink -f ./root)

CFLAGS = -Wall -Werror -Iinclude -I$(BUILD)/include
LDFLAGS = -L$(BUILD)/lib
EXTRA_CFLAGS ?= -O2
EXTRA_LDFLAGS ?=

ifdef X
export CC = $(TOOLCHAIN)/bin/arm-linux-gnueabihf-gcc
export LD = $(CC)
LDFLAGS += -L$(TOOLCHAIN)/lib
CFLAGS += -I$(TOOLCHAIN)/include
else
export CC = gcc
export LD = $(CC)
endif

LIBS = -lasound -l:libr.a
SRC = $(shell git ls-files)

DEPLOY_FB=/dev/fb0
DEPLOY_HOST=pi@192.168.1.166
DEPLOY_TARGET=$(DEPLOY_HOST):demo
DEPLOY_LOCAL_EXECUTABLE=./demo

demo: main.o demo.o renderer.o fb.o
	$(LD) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ $^ $(LIBS)

ametro: ametro.c
	$(CC) -o $@ -lasound $<

deploy: demo
	scp $< $(DEPLOY_TARGET)
	ssh $(DEPLOY_HOST) $(DEPLOY_LOCAL_EXECUTABLE) $(DEPLOY_FB)

toolchain:
	$(MAKE) -C $@ PREFIX=$(TOOLCHAIN)

libr:
	$(MAKE) -C $@ install PREFIX=$(BUILD)

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c $<

clean:
	$(MAKE) -C libr clean
	rm -rf $(BUILD) *.o demo
	rm -f *.o demo

disable_cursor:
	ssh $(DEPLOY_HOST) /bin/sh -c "echo -e '\033[?17;0;0c' | sudo tee /dev/tty1"

.PHONY: run clean toolchain deploy libr disable_cursor
