BUILD ?= ../build
CFLAGS = -Wall -Werror -I$(BUILD)/include
LDFLAGS = -L$(BUILD)/lib
LOG_LEVEL ?= 3

EXTRA_CFLAGS ?= -O3 -DLOG_LEVEL=$(LOG_LEVEL)
EXTRA_LDFLAGS ?=

ifdef PROFILE
EXTRA_CFLAGS += -pg
EXTRA_LDFLAGS += -pg
endif

TIMEOUT ?= 1m

CC = gcc
LD = $(CC)

LIBS = -lxcb -lxcb-keysyms -l:libr.a -lm
SRC = $(shell git ls-files)

run: rt
	timeout --preserve-status $(TIMEOUT) ./$< > out.ppm && feh out.ppm

gdb: main
	gdb $<

profile:
	$(MAKE) clean run PROFILE=1
	gprof main gmon.out | head -n10

rt: rt.o
	$(LD) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ $^ $(LIBS)

main: main.o rt.o
	$(LD) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c $<

clean:
	rm -f *.o main

.PHONY: run clean gdb profile
