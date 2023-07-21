VENDOR_ROOT=../../vendor

OUTPUT_NAME=bootloader

# Default target.
.PHONY: force all
all: $(OUTPUT_NAME)

C_ARGS=-target x86_64-unknown-windows -fno-stack-protector -ffreestanding -fshort-wchar -mno-red-zone -mno-mmx -mno-sse -I $(VENDOR_ROOT)/c-efi/src/ -I ../../inc/
LINK_ARGS=-target x86_64-unknown-windows -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld-link

$(VENDOR_ROOT) c-efi:
	make -C $(VENDOR_ROOT)

override CPPFILES := $(wildcard *.cpp)
override HFILES := $(shell find -L . -type f -name '*.h')
override OFILES := $(CPPFILES:.cpp=.o)

override KERNEL_OBJECTS := memory.o fat.o string.o

memory.o: ../kernel/memory/memory.cpp
	clang $(C_ARGS) $< -c -o $@

fat.o: ../kernel/fs/fat/fat.cpp
	clang $(C_ARGS) $< -c -o $@

string.o: ../kernel/common/string.cpp
	clang $(C_ARGS) $< -c -o $@

bootloader_build.h: force
	rm -f bootloader_build.h
	echo "#define BOOTLOADER_BUILD_ID u\"$(shell git rev-parse HEAD | cut -c 1-8) $(shell date '+%FT %T')\"" > bootloader_build.h

$(OUTPUT_NAME): c-efi $(CPPFILES) $(HFILES) $(KERNEL_OBJECTS) bootloader_build.h
	clang $(C_ARGS) $(CPPFILES) -c
	clang $(LINK_ARGS) $(OFILES) $(KERNEL_OBJECTS) -o $(OUTPUT_NAME)
