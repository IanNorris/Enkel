#!/bin/bash

echo Starting qemu

qemu-system-x86_64 -drive format=raw,media=cdrom,readonly=on,file=./boot_iso/enkel.iso,if=none,id=cdrom -device ahci,id=ahci -device ide-cd,bus=ahci.0,drive=cdrom -bios ./boot_iso/ovmf_bios.bin -serial mon:stdio -m 8G -smp sockets=2,cores=4 -vga virtio -display gtk,zoom-to-fit=off -d int,cpu_reset -D ./qemu_log.log -gdb tcp:localhost:5000 -no-reboot -monitor vc:1920x1080 -usb -device qemu-xhci -device usb-kbd -device usb-mouse -M q35