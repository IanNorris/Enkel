# Default target.
.PHONY: all
all: uefi-headers acpica zlib acpica_out

uefi-headers:
	git clone https://github.com/kiznit/uefi-headers.git uefi-headers

acpica:
	git clone https://github.com/acpica/acpica.git

zlib:
	git clone https://github.com/madler/zlib.git

acpica_out:
	make -C acpica_out
