CFLAGS = -Wall -Werror
LDFLAGS =
EXTRA_CFLAGS ?= -O2
EXTRA_LDFLAGS ?=

CC = gcc
LD = $(CC)

LIBS =
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
