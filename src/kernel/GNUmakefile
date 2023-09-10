# Nuke built-in rules and variables.
override MAKEFLAGS += -rR

# This is the name that our final kernel executable will have.
# Change as needed.
override KERNEL := enkel.elf

override DEBUG_FLAGS := -g3 -O0
#override DEBUG_FLAGS := -O3

# The name of the font
FONT_NAME := Jura
FONT_ROOT := ../../fonts
FONT_CPP_FILES := $(FONT_ROOT)/$(FONT_NAME).cpp $(FONT_ROOT)/$(FONT_NAME)_0.cpp
FONT_H_FILES := $(FONT_ROOT)/$(FONT_NAME).h  $(FONT_ROOT)/$(FONT_NAME)_0.h 
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
    -Wall \
    -Wextra \
    -finput-charset=CP1252\
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
    -pie \
    -fPIE \
    -mgeneral-regs-only \
    -m64 \
    -march=x86-64 \
    -mabi=sysv \
    -mno-80387 \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -mno-red-zone \
    -I $(FONT_ROOT) \
	-I ../../inc \
    -I $(VENDOR_ROOT)/uefi-headers/Include/ \
    -I $(VENDOR_ROOT)/uefi-headers/Include/X64

# Internal C preprocessor flags that should not be changed by the user.
override CPPFLAGS := \
    -I. \
    $(CFLAGS) \
    -MMD \
    -MP

# Internal linker flags that should not be changed by the user.
override LDFLAGS += \
    -m elf_x86_64 \
    -nostdlib \
    -static \
    -pie \
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

override SFLAGS += \
    -Wall \
    -fPIC

# Use "find" to glob all *.c, *.S, and *.asm files in the tree and obtain the
# object and header dependency file names.
override CPPFILES := $(shell find -L . -type f -name '*.cpp') $(FONT_CPP_FILES)
override ASFILES := $(shell find -L . -type f -name '*.S')
override NASMFILES := $(shell find -L . -type f -name '*.asm')
override NASMBINFILES := $(shell find -L . -type f -name '*.asmb')
override NASMBINFILES := $(shell find -L . -type f -name '*.asmb')
override OBJ := $(CPPFILES:.cpp=.cpp.o) $(ASFILES:.S=.S.o) $(NASMFILES:.asm=.asm.o) $(NASMBINFILES:.asmb=.asmb.o)
override HEADER_DEPS := $(CPPFILES:.cpp=.cpp.d) $(ASFILES:.S=.S.d)

# Default target.
.PHONY: force all
all: $(NASMBINFILES $(KERNEL) $(FONT_CPP_FILES) kernel_build.cpptemp.o $(FONT_H_FILES) $(NASMFILES))

$(FONT_CPP_FILES) $(FONT_H_FILES): fonts

uefi-headers:
	make -C $(VENDOR_ROOT)

kernel_build.cpptemp.o:
	rm -f kernel_build.cpptemp
	echo "const char16_t* KernelBuildId = u\"$(shell git rev-parse HEAD | cut -c 1-8) $(shell date '+%FT %T')\";\n" > kernel_build.cpptemp
	$(CC) -x c++ $(CFLAGS) $(CPPFLAGS) -c kernel_build.cpptemp -o kernel_build.cpptemp.o

fonts: 
	$(MAKE) -C ../../fonts

# Link rules for the final kernel executable.
$(KERNEL): $(OBJ) $(CPPFILES) $(LDTEMPLATE) $(NASMFILES) uefi-headers kernel_build.cpptemp.o
	$(LD) $(OBJ) kernel_build.cpptemp.o $(LDFLAGS) -o $@
	rm -f kernel_build.cpptemp
	rm -f kernel_build.cpptemp.o

# Include header dependencies.
-include $(HEADER_DEPS)

# Compilation rules for *.cpp files.
%.cpp.o: %.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Compilation rules for *.S files.
%.S.o: %.S
	$(CC) $(SFLAGS) -c $< -o $@

# Compilation rules for *.asm (nasm) files.
%.asm.o: %.asm
	nasm $(NASMFLAGS) $< -o $@

%.asmb.o: %.asmb
	nasm $(NASMBINFLAGS) $< -o $(basename $<).bin
	python3 $(TOOLS)/bin2c.py $(basename $<).bin $(basename $<)
	mv $(basename $<).h $(basename $<).htemp
	mv $(basename $<).cpp $(basename $<).cpptemp
	$(CC) -x c++ $(CFLAGS) $(CPPFLAGS) -c $(basename $<).cpptemp -o $@

# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf $(KERNEL) $(OBJ) $(HEADER_DEPS)
