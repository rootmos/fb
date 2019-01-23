install: /Makefile
	$(MAKE) 
	$(MAKE) install

$(BUILD)/Makefile:
	CC=$(PREFIX)/bin/arm-linux-gnueabihf-gcc \
		./configure \
		--prefix=$(PREFIX) \
		--host=arm-linux-gnueabihf \
		--disable-python --disable-alisp --disable-old-symbols
