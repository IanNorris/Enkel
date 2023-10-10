# Default target.
.PHONY: all
all: uefi-headers acpica qrcode acpica_out

uefi-headers:
	git clone https://github.com/kiznit/uefi-headers.git uefi-headers

acpica:
	git clone https://github.com/acpica/acpica.git

qrcode:
	git clone https://github.com/ricmoo/QRCode.git

acpica_out:
	make -C acpica_out
