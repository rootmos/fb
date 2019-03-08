BUILD ?= ../build
CFLAGS = -Wall -Werror -I$(BUILD)/include
LDFLAGS = -L$(BUILD)/lib
EXTRA_CFLAGS ?= -O2 -DLOG_LEVEL=2
EXTRA_LDFLAGS ?=

CC = gcc
LD = $(CC)

LIBS = -lxcb -lxcb-keysyms -l:libr.a -lm
SRC = $(shell git ls-files)

run: main
	timeout 10 ./$<

gdb: main
	gdb $<

main: main.o rt.o
	$(LD) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c $<

clean:
	rm -f *.o main

.PHONY: run clean gdb
