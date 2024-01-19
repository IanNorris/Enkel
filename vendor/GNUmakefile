# Default target.
.PHONY: all
all: uefi-headers acpica zlib acpica_out xxhash

uefi-headers:
	git clone https://github.com/kiznit/uefi-headers.git uefi-headers

acpica:
	git clone https://github.com/acpica/acpica.git

zlib:
	git clone https://github.com/madler/zlib.git

acpica_out:
	make -C acpica_out

xxhash:
	git clone https://github.com/Cyan4973/xxHash.git
