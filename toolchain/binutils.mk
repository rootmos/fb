BUILD=$(BUILD_ROOT)/binutils

install: $(BUILD)/Makefile
	$(MAKE) -C $(BUILD)
	$(MAKE) -C $(BUILD) install

$(BUILD)/Makefile:
	mkdir -p $(BUILD) && cd $(BUILD) && $(shell realpath ./configure) \
		--prefix=$(PREFIX) \
		--target=arm-linux-gnueabihf \
		--with-arch=armv6 --with-fpu=vfp --with-float=hard \
		--disable-multilib

.PHONY: install
