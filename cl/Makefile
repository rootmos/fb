CC = gcc
LD = $(CC)
LOG_LEVEL ?= 3

BUILD ?= ../build
CFLAGS = -Wall -Werror -DLOG_LEVEL=$(LOG_LEVEL) -I$(BUILD)/include
LDFLAGS = -L$(BUILD)/lib

EXTRA_CFLAGS ?= -g -O1
EXTRA_LDFLAGS ?=

FORMAT ?= ppm
OUTPUT ?= $(shell date -u +%Y%m%dT%H%M%SZ).$(FORMAT)

ifdef PROFILE
EXTRA_CFLAGS += -pg
EXTRA_LDFLAGS += -pg
endif

ifdef DEBUG
EXTRA_CFLAGS += -DDEBUG
endif

ifdef QUICK
EXTRA_CFLAGS += -DQUICK
endif

ifndef VIEWER
ifeq ($(FORMAT), ppm)
VIEWER = feh
else ifeq ($(FORMAT), mkv)
VIEWER = mpv
endif
endif

LIBS = -l:libr.a -lm -lOpenCL -lavcodec -lavutil -lavformat

ppm mkv:
	$(MAKE) run FORMAT=$@

run: main
ifdef TIMEOUT
	timeout --preserve-status $(TIMEOUT) ./$< $(OUTPUT)
else
	./$< $(OUTPUT)
endif
ifndef DEBUG
	$(VIEWER) $(OUTPUT)
endif

gdb: main
	gdb $<

profile:
	$(MAKE) clean run PROFILE=1
	gprof main gmon.out | head -n10

SRC=main.c
AUX=rt.cl rt.c shared.h types.h types.cl rnd.cl world.c enc.c
main: $(SRC) $(AUX)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ $(SRC) $(LIBS)

clean:
	rm -f main out.* *.mkv *.ppm

.PHONY: ppm mkv
.PHONY: run clean gdb profile
