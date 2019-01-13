install:
	KERNEL=kernel7 $(MAKE) ARCH=arm INSTALL_HDR_PATH=$(PREFIX)/arm-linux-gnueabihf headers_install

.PHONY: install
