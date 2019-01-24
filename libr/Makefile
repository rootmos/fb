CC ?= cc
LD ?= $(CC)
CFLAGS = -Wall -Werror -I./include
EXTRA_CFLAGS ?= -O2
EXTRA_LDFLAGS ?=
PREFIX ?= /usr

libr.a: util.o
	ar -cr $@ $^

install: libr.a
	install -d $(PREFIX)/include/r
	install -m 644 include/*.h $(PREFIX)/include
	install -m 644 include/r/*.h $(PREFIX)/include/r
	install -d $(PREFIX)/lib
	install -m 644 $< $(PREFIX)/lib

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c $<

clean:
	rm -rf *.o *.a

.PHONY: install clean
