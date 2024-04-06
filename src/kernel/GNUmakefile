# Nuke built-in rules and variables.
override MAKEFLAGS += -rR

# This is the name that our final kernel executable will have.
# Change as needed.
override KERNEL := enkel.elf

override DEBUG_FLAGS := -ggdb3 -O0 -DDEBUG -D_DEBUG -DACPI_DEBUG_OUTPUT -DENABLE_STATISTICS=1 -DVERBOSE_LOGGING=0
#override DEBUG_FLAGS := -O3

# The name of the font
FONT_NAME := CascadiaMono
FONT_ROOT := ../../fonts
FONT_CPP_FILES := $(FONT_ROOT)/$(FONT_NAME).cpp $(FONT_ROOT)/$(FONT_NAME)_0.cpp
FONT_H_FILES := $(FONT_ROOT)/$(FONT_NAME).h  $(FONT_ROOT)/$(FONT_NAME)_0.h
EMBEDDED_IMAGE_FILES := ../../assets/SplashLogo.tga
EMBEDDED_IMAGE_CPP_FILES := ../../assets/SplashLogo.cpp
EMBEDDED_IMAGE_H_FILES := ../../assets/SplashLogo.h
EMBEDDED_APPS := ../apps/hello_world/hello_world.a
EMBEDDED_APP_DIRECTORIES := ../apps/hello_world
override EMBEDDED_APP_BINCPP_FILES := $(EMBEDDED_APPS:.a=.bin.cpp)
override EMBEDDED_APP_BINH_FILES := $(EMBEDDED_APPS:.a=.bin.h)
TOOLS := ../../tools

LDTEMPLATE := linker.ld

# Convenience macro to reliably declare user overridable variables.
define DEFAULT_VAR =
    ifeq ($(origin $1),default)
        override $(1) := $(2)
    endif
    ifeq ($(origin $1),undefined)
        override $(1) := $(2)
    endif
endef

VENDOR_ROOT := ../../vendor

RPMALLOC_C_FILES := $(VENDOR_ROOT)/rpmalloc/rpmalloc/rpmalloc.c $(VENDOR_ROOT)/rpmalloc/rpmalloc/malloc.c
QRCODE_C_FILES := $(VENDOR_ROOT)/QR-Code-generator/c/qrcodegen.c
ZLIB_C_FILES := $(VENDOR_ROOT)/zlib/adler32.c $(VENDOR_ROOT)/zlib/compress.c $(VENDOR_ROOT)/zlib/crc32.c $(VENDOR_ROOT)/zlib/deflate.c $(VENDOR_ROOT)/zlib/infback.c $(VENDOR_ROOT)/zlib/inffast.c $(VENDOR_ROOT)/zlib/inflate.c $(VENDOR_ROOT)/zlib/inftrees.c $(VENDOR_ROOT)/zlib/trees.c $(VENDOR_ROOT)/zlib/uncompr.c $(VENDOR_ROOT)/zlib/zutil.c
STM32_MW_FATFS_C_FILES := $(VENDOR_ROOT)/stm32_mw_fatfs-master/source/ff.c $(VENDOR_ROOT)/stm32_mw_fatfs-master/source/ffunicode.c
XXHASH_C_FILES := $(VENDOR_ROOT)/xxHash/xxhash.c

# It is highly recommended to use a custom built cross toolchain to build a kernel.
# We are only using "cc" as a placeholder here. It may work by using
# the host system's toolchain, but this is not guaranteed.
override DEFAULT_CC := cc
$(eval $(call DEFAULT_VAR,CC,$(DEFAULT_CC)))

# Same thing for "ld" (the linker).
override DEFAULT_LD := ld
$(eval $(call DEFAULT_VAR,LD,$(DEFAULT_LD)))

# User controllable C flags.
override DEFAULT_CFLAGS := -g -O2 -pipe
$(eval $(call DEFAULT_VAR,CFLAGS,$(DEFAULT_CFLAGS)))

# User controllable C preprocessor flags. We set none by default.
override DEFAULT_CPPFLAGS :=
$(eval $(call DEFAULT_VAR,CPPFLAGS,$(DEFAULT_CPPFLAGS)))

# User controllable nasm flags.
override DEFAULT_NASMFLAGS := -F dwarf -g
$(eval $(call DEFAULT_VAR,NASMFLAGS,$(DEFAULT_NASMFLAGS)))

# User controllable linker flags. We set none by default.
override DEFAULT_LDFLAGS :=
$(eval $(call DEFAULT_VAR,LDFLAGS,$(DEFAULT_LDFLAGS)))

# Internal C flags that should not be changed by the user.
# -mgeneral-regs-only for interrupt handling (for now, see https://wiki.osdev.org/Interrupt_Service_Routines)
override CFLAGS += \
    $(DEBUG_FLAGS) \
	-DENABLE_NX=0 \
	-D__ENKEL__ \
	-DARCH_64BIT \
	-DRPMALLOC_CONFIGURABLE=1 \
    -Wall \
    -Wextra \
	-Wno-unused-parameter \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
    -mgeneral-regs-only \
    -m64 \
    -march=x86-64 \
    -mabi=sysv \
    -mno-80387 \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -mno-red-zone \
	-mcmodel=large \
    -I $(FONT_ROOT) \
	-I ../../inc \
    -I $(VENDOR_ROOT)/uefi-headers/Include/ \
    -I $(VENDOR_ROOT)/uefi-headers/Include/X64 \
	-I $(VENDOR_ROOT)/acpica/source/include \
	-I $(VENDOR_ROOT)/rpmalloc/rpmalloc \
	-I $(VENDOR_ROOT)/QR-Code-generator/c/ \
	-I $(VENDOR_ROOT)/zlib/ \
	-I $(VENDOR_ROOT)/stm32_mw_fatfs-master/source/ \
	-I $(VENDOR_ROOT)/xxHash/

# Internal C preprocessor flags that should not be changed by the user.
override CPPFLAGS := \
    $(CFLAGS) \
    -MMD \
    -MP

# Internal linker flags that should not be changed by the user.
override LDFLAGS += \
    -m elf_x86_64 \
    -nostdlib \
    -static \
    --no-dynamic-linker \
    -z text \
    -z max-page-size=0x1000 \
    -e KernelMain \
    -T $(LDTEMPLATE)

override NASMFLAGS += \
    -Wall \
    -f elf64

override NASMBINFLAGS += \
    -Wall \
    -f bin

# Use "find" to glob all *.c, and *.asm files in the tree and obtain the
# object and header dependency file names.
override CPPFILES := $(shell find -L . -type f -name '*.cpp') $(FONT_CPP_FILES) $(EMBEDDED_IMAGE_CPP_FILES) $(EMBEDDED_APP_BINCPP_FILES)
override CFILES := $(RPMALLOC_C_FILES) $(QRCODE_C_FILES) $(ZLIB_C_FILES) $(STM32_MW_FATFS_C_FILES) $(XXHASH_C_FILES)
override NASMFILES := $(shell find -L . -type f -name '*.asm')
override NASMBINFILES := $(shell find -L . -type f -name '*.asmb')
override NASMBINFILES := $(shell find -L . -type f -name '*.asmb')
override OBJ := $(CPPFILES:.cpp=.cpp.o) $(CFILES:.c=.c.o) $(NASMFILES:.asm=.asm.o) $(NASMBINFILES:.asmb=.asmb.o)
override HEADER_DEPS := $(CPPFILES:.cpp=.cpp.d) $(CFILES:.c=.c.d)

# Default target.
.PHONY: force all
all: vendor embedded_assets embedded_apps $(VENDOR_ROOT)/acpica_out/acpica.a $(NASMBINFILES) $(KERNEL) kernel_build.cpptemp.o $(NASMFILES) $(FONT_CPP_FILES) $(FONT_H_FILES) $(EMBEDDED_IMAGE_CPP_FILES) $(EMBEDDED_IMAGE_H_FILES) $(QRCODE_C_FILES) $(STM32_MW_FATFS_C_FILES) $(EMBEDDED_APP_BINCPP_FILES)

vendor:
	make -C $(VENDOR_ROOT)

embedded_apps $(EMBEDDED_APP_BINCPP_FILES) $(EMBEDDED_APP_BINH_FILES): $(EMBEDDED_APP_DIRECTORIES)
	make -C $<

embedded_assets:
	$(MAKE) -C ../../assets

$(VENDOR_ROOT)/acpica_out/acpica.a:
	make -C $(VENDOR_ROOT)/acpica_out

uefi-headers:
	make -C $(VENDOR_ROOT)

kernel_build.cpptemp.o:
	rm -f kernel_build.cpptemp
	echo "const char16_t* KernelBuildId = u\"$(shell date '+%F') $(shell git rev-parse HEAD | cut -c 1-8)\";" > kernel_build.cpptemp
	$(CC) -x c++ $(CFLAGS) $(CPPFLAGS) -c kernel_build.cpptemp -o kernel_build.cpptemp.o

$(FONT_CPP_FILES) $(FONT_H_FILES):
	$(MAKE) -C ../../fonts

# Link rules for the final kernel executable.
$(KERNEL): $(VENDOR_ROOT)/acpica_out/acpica.a $(NASMBINFILES:.asmb=.asmb.o) $(OBJ) $(CPPFILES) $(CFILES) $(LDTEMPLATE) $(NASMFILES) uefi-headers kernel_build.cpptemp.o
	$(LD) $(OBJ) kernel_build.cpptemp.o -L $(VENDOR_ROOT)/acpica_out -lacpica $(LDFLAGS) -Map=$(basename $@).map -o $@
	rm -f kernel_build.cpptemp
	rm -f kernel_build.cpptemp.o

# Include header dependencies.
-include $(HEADER_DEPS)

# Compilation rules for *.cpp files.
%.cpp.o: %.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

%.c.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compilation rules for *.asm (nasm) files.
%.asm.o: %.asm
	nasm $(NASMFLAGS) -l $(basename $<).lst $< -o $@

%.asmb.o: %.asmb
	nasm $(NASMBINFLAGS) -l $(basename $<).lst $< -o $(basename $<).bin
	python3 $(TOOLS)/bin2c.py $(basename $<).bin $(basename $<)
	mv $(basename $<).h $(basename $<).htemp
	mv $(basename $<).cpp $(basename $<).cpptemp
	$(CC) -x c++ $(CFLAGS) $(CPPFLAGS) -c $(basename $<).cpptemp -o $@

# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf $(KERNEL) $(OBJ) $(HEADER_DEPS)
