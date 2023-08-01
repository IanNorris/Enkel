section .data

global SetGDT
global ReloadSegments
align 4

GDTRegister  DW 0 ; GDT limit
             DQ 0 ; GDT base

section .text

SetGDT:
    mov [rel GDTRegister], DI
    mov [rel GDTRegister+2], RSI
    lgdt [rel GDTRegister]
    ret

ReloadSegments:
    push 0x08 ; CS register offset
    lea rax, [rel Reload]
    push rax
    retfq ; This jumps to Reload

Reload: mov ax, 0x10 ; DS register offset
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ret