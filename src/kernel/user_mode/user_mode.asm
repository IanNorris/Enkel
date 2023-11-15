section .text
global SwitchToUserMode
global ReturnToKernel

extern NextTask

SwitchToUserMode:
	;   rdi - uint64_t stackPointer (user mode stack pointer)
	;   rsi - uint64_t entry (user mode entry point address)
	;	rdx - uint16_t userModeCS
	;	rcx - uint16_t userModeDS

	push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    pushfq

	push rax

	; NextTask is our target memory location
	mov rax, NextTask
	mov [rax], rbp

	;add rax, 8
	mov [rax+8], rsp

	pop rax

    ; Clear the direction flag
    cld

	push 0x08 ; CS register offset
	push rdi

	push rcx ;UM DS
	push rdi ;UM stack
	push 0x202 ; rflags
	push rdx ;UM CS
	push rsi ;UM entry

    ; Switch to user mode by performing a far return
    iretq

ReturnToKernel:

	lea rbp, [NextTask + 0]
	lea rsp, [NextTask + 8]

	; This matches the pop we made earlier
	; so we could manipulate the task pointer
	pop rax

	popfq
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
	
	ret
