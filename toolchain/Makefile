# Following: https://solarianprogrammer.com/2018/05/06/building-gcc-cross-compiler-raspberry-pi/

MIRROR=https://ftpmirror.gnu.org
BINUTILS_TARBALL=binutils-2.31.1.tar.gz
BINUTILS_URL=$(MIRROR)/gnu/binutils/$(BINUTILS_TARBALL)
GCC_TARBALL=gcc-8.2.0.tar.gz
GCC_URL=$(MIRROR)/gnu/gcc/gcc-8.2.0/$(GCC_TARBALL)
GLIBC_TARBALL=glibc-2.28.tar.gz
GLIBC_URL=$(MIRROR)/glibc/$(GLIBC_TARBALL)
ALSA_TARBALL=alsa-lib-1.0.27.tar.bz2
ALSA_URL=ftp://ftp.alsa-project.org/pub/lib/$(ALSA_TARBALL)

LINUX_TARBALL=raspberrypi-kernel_1.20181112-1.tar.gz
LINUX_URL=https://github.com/raspberrypi/linux/archive/$(LINUX_TARBALL)

export PREFIX=$(shell realpath root)
export BUILD_ROOT=$(shell realpath build)
export PATH:=$(PREFIX)/bin:$(PATH)

FETCH=wget -O

all:
	$(MAKE) install-linux
	$(MAKE) install-binutils
	$(MAKE) partial-gcc
	$(MAKE) partial-glibc
	$(MAKE) install-libgcc
	$(MAKE) install-glibc
	$(MAKE) install-gcc
	$(MAKE) install-alsa

# binutils
###############################################################################

$(BINUTILS_TARBALL):
	$(FETCH) $@ $(BINUTILS_URL)

binutils: $(BINUTILS_TARBALL)
	mkdir -p $@
	tar -xf $< --strip-components=1 -C $@

# gcc
###############################################################################

$(GCC_TARBALL):
	$(FETCH) $@ $(GCC_URL)

gcc: $(GCC_TARBALL)
	mkdir -p $@
	tar -xf $< --strip-components=1 -C $@

install-libgcc:
	$(MAKE) -f ../gcc.mk -C gcc libgcc

# glibc
###############################################################################

$(GLIBC_TARBALL):
	$(FETCH) $@ $(GLIBC_URL)

glibc: $(GLIBC_TARBALL)
	mkdir -p $@
	tar -xf $< --strip-components=1 -C $@

# linux
###############################################################################

$(LINUX_TARBALL):
	$(FETCH) $@ $(LINUX_URL)

linux: $(LINUX_TARBALL)
	tar -xf $< --transform='s,^[^/]\+/,linux/,'

# alsa
###############################################################################

$(ALSA_TARBALL):
	$(FETCH) $@ $(ALSA_URL)

alsa: $(ALSA_TARBALL)
	mkdir -p $@
	tar -xf $< --strip-components=1 -C $@

# misc
###############################################################################

partial-%: %
	$(MAKE) -f ../$<.mk -C $< partial

install-%: %
	$(MAKE) -f ../$<.mk -C $< install

clean:
	rm -rf root build gcc linux binutils glibc

.PHONY: all clean
