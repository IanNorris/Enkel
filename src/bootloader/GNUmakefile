VENDOR_ROOT=../../vendor

OUTPUT_NAME=bootloader

# Default target.
.PHONY: all
all: $(OUTPUT_NAME)

C_ARGS=-target x86_64-unknown-windows -fno-stack-protector -ffreestanding -fshort-wchar -mno-red-zone -mno-mmx -mno-sse -I $(VENDOR_ROOT)/c-efi/src/ -I $(VENDOR_ROOT)/fat16/ -I ../../inc/
LINK_ARGS=-target x86_64-unknown-windows -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld-link

$(VENDOR_ROOT) c-efi:
	make -C $(VENDOR_ROOT)

override CFILES := $(shell find -L . -type f -name '*.c')
override HFILES := $(shell find -L . -type f -name '*.h')
override OFILES := $(CFILES:.c=.c.o)

memory.o: ../kernel/memory/memory.c
	clang $(C_ARGS) $(CFILES) -c -o memory.o

bootloader_build.h:
	echo "#define BOOTLOADER_BUILD_ID L\"$(shell git rev-parse HEAD | cut -c 1-8) $(shell date '+%FT %T')\"" > bootloader_build.h

$(OUTPUT_NAME): c-efi $(CFILES) $(HFILES) bootloader_build.h
	clang $(C_ARGS) $(CFILES) -c
	clang $(LINK_ARGS) $(shell find -L . -type f -name '*.o') memory.o -o $(OUTPUT_NAME)
	rm bootloader_build.h