CC = gcc
LD = $(CC)
LOG_LEVEL ?= 5

BUILD ?= ../build
CFLAGS = -Wall -Werror -DLOG_LEVEL=$(LOG_LEVEL) -I$(BUILD)/include
LDFLAGS = -L$(BUILD)/lib

EXTRA_CFLAGS ?= -O1
EXTRA_LDFLAGS ?=

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

TIMEOUT ?= 20s

LIBS = -l:libr.a -lm -lOpenCL

run: main
ifdef DEBUG
	timeout --preserve-status $(TIMEOUT) ./$<
else
	timeout --preserve-status $(TIMEOUT) ./$< > out.ppm && feh out.ppm
endif

gdb: main
	gdb $<

profile:
	$(MAKE) clean run PROFILE=1
	gprof main gmon.out | head -n10

SRC=main.c rt.c
AUX=rt.cl rt.h shared.h types.h types.cl rnd.cl world.c
main: $(OBJS) $(AUX)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ $(SRC) $(LIBS)

clean:
	rm -f main out.ppm

.PHONY: run clean gdb profile
