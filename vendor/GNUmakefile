# Default target.
.PHONY: all
all: uefi-headers acpica zlib\adler32.o acpica_out xxHash/xxhash.h

uefi-headers:
	if [ ! -d "uefi-headers" ]; then git clone https://github.com/kiznit/uefi-headers.git uefi-headers; fi

acpica:
	if [ ! -d "acpica" ]; then git clone https://github.com/acpica/acpica.git; fi

zlib\zlib.h:
	if [ ! -d "zlib" ]; then git clone https://github.com/madler/zlib.git; fi

xxHash/xxhash.h:
	if [ ! -d "xxHash" ]; then git clone https://github.com/Cyan4973/xxHash.git; fi

zlib\adler32.o: zlib\zlib.h
	make -C zlib

acpica_out:
	make -C acpica_out

xxHash/xxhash.h:
	git clone https://github.com/Cyan4973/xxHash.git
