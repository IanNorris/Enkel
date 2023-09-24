
override DEBUG_FLAGS := -g3 -O0 -D_DEBUG -DACPI_DEBUGGER
#override DEBUG_FLAGS := -O3

ACPICA_ROOT := ../acpica
ACPICA_SOURCE := $(ACPICA_ROOT)/source

# Default target.
.PHONY: force all
all: $(ACPICA_SOURCE)/include/platform/acenkel.h libacpica.a

# Convenience macro to reliably declare user overridable variables.
define DEFAULT_VAR =
    ifeq ($(origin $1),default)
        override $(1) := $(2)
    endif
    ifeq ($(origin $1),undefined)
        override $(1) := $(2)
    endif
endef

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
	-D__ENKEL__ \
    -Wall \
	-Wno-unused-parameter \
	-Wno-unused-but-set-variable \
	-Wno-unused-function \
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
    -I $(ACPICA_SOURCE)/include

# Internal C preprocessor flags that should not be changed by the user.
override CPPFLAGS := \
    -I. \
    $(CFLAGS) \
    -MMD \
    -MP

# Use "find" to glob all *.c, *.S, and *.asm files in the tree and obtain the
# object and header dependency file names.
override CFILES := $(shell find -L $(ACPICA_SOURCE)/components -type f -name '*.c')
override OBJ := $(CFILES:.c=.c.o)
override HEADER_DEPS := $(CFILES:.c=.c.d)

libacpica.a: $(OBJ)
	ar rcs $@ $(OBJ)
	ranlib $@

# Include header dependencies.
-include $(HEADER_DEPS)

$(ACPICA_SOURCE)/include/platform/acenkel.h: acenkel.h acenv.h
	cp -f acenv.h $(ACPICA_SOURCE)/include/platform/
	cp -f acenkel.h $(ACPICA_SOURCE)/include/platform/

# Compilation rules for *.c files.
%.c.o: %.c $(ACPICA_SOURCE)/include/platform/acenkel.h $(ACPICA_SOURCE)/include/platform/acenv.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf $(KERNEL) $(OBJ) $(HEADER_DEPS)
