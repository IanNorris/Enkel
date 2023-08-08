VENDOR_ROOT=../../vendor

OUTPUT_NAME=bootloader

# Default target.
.PHONY: force all
all: $(OUTPUT_NAME)

C_ARGS=-target x86_64-unknown-windows -fno-stack-protector -ffreestanding -fshort-wchar -mno-red-zone -mno-mmx -mno-sse -I $(VENDOR_ROOT)/uefi-headers/Include/ -I $(VENDOR_ROOT)/uefi-headers/Include/X64 -I ../../inc/ -I $(VENDOR_ROOT)/elf/
LINK_ARGS=-target x86_64-unknown-windows -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld-link

# Feels odd setting win64 here, but it's for compatibility with the EFI binary
override NASMFLAGS += -Wall -f win64

uefi-headers:
	make -C $(VENDOR_ROOT)

override CPPFILES := $(wildcard *.cpp)
override HFILES := $(shell find -L . -type f -name '*.h')
override NASMFILES := $(shell find -L . -type f -name '*.asm')
override OFILES := $(CPPFILES:.cpp=.o)

override KERNEL_OBJECTS := memory.o fat.o string.o

memory.o: ../kernel/memory/memory.cpp
	clang $(C_ARGS) $< -c -o $@

fat.o: ../kernel/fs/fat/fat.cpp
	clang $(C_ARGS) $< -c -o $@

string.o: ../kernel/common/string.cpp
	clang $(C_ARGS) $< -c -o $@

# Compilation rules for *.asm (nasm) files.
%.asm.o: %.asm
	nasm $(NASMFLAGS) $< -o $@

bootloader_build.h: force
	rm -f bootloader_build.h
	echo "#define BOOTLOADER_BUILD_ID u\"$(shell git rev-parse HEAD | cut -c 1-8) $(shell date '+%FT %T')\"" > bootloader_build.h

$(OUTPUT_NAME): uefi-headers $(CPPFILES) $(HFILES) $(KERNEL_OBJECTS) $(NASMFILES:.asm=.asm.o) bootloader_build.h
	clang $(C_ARGS) $(CPPFILES) -c
	clang $(LINK_ARGS) $(OFILES) $(NASMFILES:.asm=.asm.o) $(KERNEL_OBJECTS) -o $(OUTPUT_NAME)
