section .data

global ReloadSegments
global SwitchToUserMode
global LoadTSS
align 4

section .text

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

LoadTSS:
	mov ax, di
    ltr ax
    ret