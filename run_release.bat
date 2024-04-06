@echo off

start /B "" "C:\Program Files\qemu\qemu-system-x86_64.exe" -drive format=raw,media=cdrom,readonly=on,file=./boot_iso/enkel.iso -bios ./boot_iso/ovmf_bios.bin -m 8G -smp sockets=2,cores=4 -vga virtio -display gtk,zoom-to-fit=off -monitor vc:1920x1080 -usb -M q35
