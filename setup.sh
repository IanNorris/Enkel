#!/bin/bash

# Install pre-requisite packages
apt install make mtools genisoimage gnu-efi clang lld gcc g++ nasm qemu ovmf qemu-system-x86 gdb

# Needed for Fedora based OSes:
# dnf install qemu-system-x86_64 qemu-ui-gtk