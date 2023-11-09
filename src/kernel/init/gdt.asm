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

SwitchToUserMode:
	;   rdi - uint64_t stackPointer (user mode stack pointer)
	;   rsi - uint64_t entry (user mode entry point address)
	;	rdx - uint16_t userModeCS
	;	rcx - uint16_t userModeDS

    ; Clear the direction flag
    cld

	push 0x08 ; CS register offset
    push rsp

	push rcx
	push rdi
	push 0x202
	push rdx
	push rsi

    ; Switch to user mode by performing a far return
    iretq

LoadTSS:
	mov ax, di
    ltr ax
    ret