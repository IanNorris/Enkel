
# See also: https://github.com/syzdek/efibootiso

ISO_LABEL=ENKEL
BOOT_IMAGE=efi.img
OUTPUT_ISO=enkel.iso
BIOS_BIN=ovmf_bios.bin
BOOT_PART=boot_part
SECTOR_SIZE=512
INFLATED_IMAGE_KB=256

GLIBC_ROOT=/lib/x86_64-linux-gnu/#Debian based (eg Ubuntu)
#GLIBC_ROOT=/usr/lib64/#Fedora based

BOOTLOADER_ROOT=../src/bootloader
BOOTLOADER=$(BOOTLOADER_ROOT)/bootloader.img
KERNEL_ROOT=../src/kernel
KERNEL=$(KERNEL_ROOT)/enkel.elf
BOOTLOADER_OUT_DIR=$(BOOT_PART)/efi/boot
BOOTLOADER_OUT=$(BOOTLOADER_OUT_DIR)/bootx64.efi
KERNEL_OUT_DIR=$(BOOT_PART)/kernel
KERNEL_OUT=$(KERNEL_OUT_DIR)/enkel.elf

# Default target.
.PHONY: all
all: $(OUTPUT_ISO)

$(BOOTLOADER):
	make -C $(BOOTLOADER_ROOT)

$(KERNEL):
	make -C $(KERNEL_ROOT)
	
tls_test:
	make -C ../src/apps/tls_test

libc_test:
	make -C ../src/apps/libc_test

static_libc_test:
	make -C ../src/apps/static_libc_test

$(BOOTLOADER_OUT): $(BOOTLOADER)
	mkdir -p $(BOOTLOADER_OUT_DIR)
	cp $(BOOTLOADER) $(BOOTLOADER_OUT)

$(KERNEL_OUT): $(KERNEL)
	mkdir -p $(KERNEL_OUT_DIR)
	cp $(KERNEL) $(KERNEL_OUT)

$(BOOT_IMAGE): $(BOOTLOADER_OUT) $(KERNEL_OUT) tls_test libc_test static_libc_test
	#Calculate the actual size of the boot partition folder

	mkdir -p $(BOOT_PART)/lib64
	mkdir -p $(BOOT_PART)/usr/lib/x86_64-linux-gnu/
	cp $(GLIBC_ROOT)/libc.so.6 $(BOOT_PART)/lib64/
	cp $(GLIBC_ROOT)/libc.so $(BOOT_PART)/lib64/
	cp $(GLIBC_ROOT)/libc.so.6 $(BOOT_PART)/lib/x86_64-linux-gnu/
	cp $(GLIBC_ROOT)/libc.so $(BOOT_PART)/lib/x86_64-linux-gnu/
	cp $(GLIBC_ROOT)/libc.so $(BOOT_PART)/usr/lib/x86_64-linux-gnu/
	cp /bin/echo $(BOOT_PART)/
	cp $(GLIBC_ROOT)/ld-linux-x86-64.so.2 $(BOOT_PART)/lib64/
	cp $(GLIBC_ROOT)/libstdc++.so.6 $(BOOT_PART)/lib64/
	cp $(GLIBC_ROOT)/libm.so.6 $(BOOT_PART)/lib64/
	cp $(GLIBC_ROOT)/libgcc_s.so.1 $(BOOT_PART)/lib64/
	cp ../assets/AnotherLogo.tga $(BOOT_PART)/Logo.tga
	cp ../src/apps/hello_world/hello_world.a $(BOOT_PART)/hello_world
	cp ../src/apps/tls_test/tls_test.a $(BOOT_PART)/tls_test
	cp ../src/apps/libc_test/libc_test.a $(BOOT_PART)/libc_test
	cp ../src/apps/static_libc_test/static_libc_test.a $(BOOT_PART)/static_libc_test

	$(eval BOOT_PART_SIZE := $(shell du -cb $(BOOT_PART) | tail -n 1 | awk '{print $$1}'))
	#Now calculate how many sectors we need by inflating it by about 30% and adding 512kb on top.
	$(eval BOOT_PART_SIZE_SECTORS := $(shell expr $(shell expr $(shell expr $(BOOT_PART_SIZE) \* 130) + $(shell expr $(INFLATED_IMAGE_KB) \* 1024 \* 100)) / $(shell expr 100 \* $(SECTOR_SIZE))))
	# Size needed is $(BOOT_PART_SIZE_SECTORS) sectors.

	dd if=/dev/zero of=$(BOOT_IMAGE) bs=$(SECTOR_SIZE) count=$(BOOT_PART_SIZE_SECTORS)
	mkfs.fat -F 12 -n '$(ISO_LABEL)' $(BOOT_IMAGE)
	mcopy -s -i $(BOOT_IMAGE) $(BOOT_PART)/* ::
	
$(BIOS_BIN):
	cp /usr/share/OVMF/OVMF_CODE.fd $(BIOS_BIN)
	
$(OUTPUT_ISO): $(BOOT_IMAGE) $(BIOS_BIN) $(KERNEL) $(BOOTLOADER)
	cp efi.img iso_content/
	mkisofs \
		-o $(OUTPUT_ISO) \
		-input-charset utf-8 \
		-R -J \
		-x $(OUTPUT_ISO) \
		-hide-rr-moved \
		-no-emul-boot \
		-e $(BOOT_IMAGE) \
		-V "$(ISO_LABEL)" \
		-A "$(ISO_LABEL)" \
		iso_content