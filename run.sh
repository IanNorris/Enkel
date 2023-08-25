#!/bin/bash

echo Starting qemu

qemu-system-x86_64 ./boot_iso/enkel.iso -bios ./boot_iso/ovmf_bios.bin -m 2G -vga virtio -display gtk,zoom-to-fit=off -d int,cpu_reset -D ./qemu_log.log -gdb tcp:localhost:5000