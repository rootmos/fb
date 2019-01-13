BUILD=$(BUILD_ROOT)/glibc

partial: $(BUILD)/Makefile
	$(MAKE) -C $(BUILD) install-bootstrap-headers=yes install-headers
	$(MAKE) -C $(BUILD) csu/subdir_lib
	install $(BUILD)/csu/crt1.o $(BUILD)/csu/crti.o $(BUILD)/csu/crtn.o $(PREFIX)/arm-linux-gnueabihf/lib
	arm-linux-gnueabihf-gcc -nostdlib -nostartfiles -shared -x c /dev/null -o $(PREFIX)/arm-linux-gnueabihf/lib/libc.so
	touch $(PREFIX)/arm-linux-gnueabihf/include/gnu/stubs.h

install: $(BUILD)/Makefile
	$(MAKE) -C $(BUILD)
	$(MAKE) -C $(BUILD) install

$(BUILD)/Makefile:
	mkdir -p $(BUILD) && cd $(BUILD) && $(shell realpath ./configure) \
		--prefix=$(PREFIX)/arm-linux-gnueabihf \
		--build=$(MACHTYPE) --host=arm-linux-gnueabihf --target=arm-linux-gnueabihf \
		--with-arch=armv6 --with-fpu=vfp --with-float=hard \
		--with-headers=$(PREFIX)/arm-linux-gnueabihf/include --disable-multilib \
		libc_cv_forced_unwind=yes

.PHONY: partial install
