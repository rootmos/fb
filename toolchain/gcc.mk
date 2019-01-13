BUILD=$(BUILD_ROOT)/gcc

partial: $(BUILD)/Makefile
	$(MAKE) -C $(BUILD) all-gcc
	$(MAKE) -C $(BUILD) install-gcc

libgcc: $(BUILD)/Makefile
	$(MAKE) -C $(BUILD) all-target-libgcc
	$(MAKE) -C $(BUILD) install-target-libgcc

install: $(BUILD)/Makefile
	$(MAKE) -C $(BUILD)
	$(MAKE) -C $(BUILD) install

$(BUILD)/Makefile:
	mkdir -p $(BUILD) && cd $(BUILD) && $(shell realpath ./configure) \
		--prefix=$(PREFIX) \
		--target=arm-linux-gnueabihf --enable-languages=c \
		--with-arch=armv6 --with-fpu=vfp --with-float=hard --disable-multilib

.PHONY: partial install libgcc
