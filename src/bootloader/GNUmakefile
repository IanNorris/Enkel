VENDOR_ROOT=../../vendor/

OUTPUT_NAME=bootloader

# Default target.
.PHONY: all
all: $(OUTPUT_NAME)

C_ARGS=-target x86_64-unknown-windows -fno-stack-protector -ffreestanding -fshort-wchar -mno-red-zone -mno-mmx -mno-sse -I $(VENDOR_ROOT)/c-efi/src/
LINK_ARGS=-target x86_64-unknown-windows -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld-link

$(VENDOR_ROOT) c-efi:
	make -C $(VENDOR_ROOT)

override CFILES := $(shell find -L . -type f -name '*.c')
override OFILES := $(shell find -L . -type f -name '*.o')

$(OUTPUT_NAME): c-efi $(CFILES)
	clang $(C_ARGS) $(CFILES) -c
	clang $(LINK_ARGS) $(OFILES) -o $(OUTPUT_NAME)