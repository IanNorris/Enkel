# PREP:
# sudo apt install mtools genisoimage

# See also: https://github.com/syzdek/efibootiso

ISOLABEL=ENKEL
BOOT_IMAGE=efi.img
OUTPUT_ISO=enkel.iso
BIOS_BIN=ovmf_bios.bin

# Default target.
.PHONY: all
all: $(OUTPUT_ISO)

$(BOOT_IMAGE): boot_part/efi/boot/bootx64.efi
	dd if=/dev/zero of=$(BOOT_IMAGE) bs=512 count=2880
	mkfs.fat -n '$(ISO_LABEL)' $(BOOT_IMAGE)
	mcopy -s -i $(BOOT_IMAGE) boot_part/* ::
	
$(BIOS_BIN):
	cp /usr/share/ovmf/OVMF.fd $(BIOS_BIN)
	
$(OUTPUT_ISO): $(BOOT_IMAGE) $(BIOS_BIN)
	cp efi.img iso_content/
	mkisofs \
		-o $(OUTPUT_ISO) \
		-input-charset utf-8 \
		-R -J -v -d -N \
		-x $(OUTPUT_ISO) \
		-hide-rr-moved \
		-no-emul-boot \
		-e $(BOOT_IMAGE) \
		-V "$(ISOLABEL)" \
		-A "Enkel Image"  \
		iso_content