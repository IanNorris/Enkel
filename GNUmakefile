# Default target.
.PHONY: all
all: kernel bootloader iso

kernel:
	make -C src/kernel

bootloader:
	make -C src/bootloader
	
iso:
	make -C boot_iso