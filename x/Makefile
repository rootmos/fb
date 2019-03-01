BUILD ?= ../build
CFLAGS = -Wall -Werror -I$(BUILD)/include
LDFLAGS = -L$(BUILD)/lib
EXTRA_CFLAGS ?= -O2
EXTRA_LDFLAGS ?=

CC = gcc
LD = $(CC)

LIBS = -lxcb -lxcb-keysyms -l:libr.a
SRC = $(shell git ls-files)

run: main
	./$<

main: main.o
	$(LD) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c $<

clean:
	rm -f *.o main

.PHONY: run clean
