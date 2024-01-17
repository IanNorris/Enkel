section .text
global SwitchToUserMode
global ReturnToKernel

extern NextTask

SwitchToUserMode:
	;   rdi - uint64_t stackPointer (user mode stack pointer)
	;   rsi - uint64_t entry (user mode entry point address)
	;	rdx - uint16_t userModeCS
	;	rcx - uint16_t userModeDS
	;	 r8 - argc
	;	 r9 - argv
	;	[rsp + (1*8)] - envp

	push r15
	mov r15, [rsp+(2*8)] ;envp

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

	;mov rbp, 0 ; set the stack base
	mov rbp, rdi

	push rcx ;UM DS
	push rdi ;UM stack
	push 0x202 ; rflags
	push rdx ;UM CS
	push rsi ;UM entry

	mov rdi, r8 ; argc
	mov rsi, r9 ; argv
	mov rdx, r15 ; envp
	mov rcx, 0

	; Clear all unused registers to 0
	; so the program doesn't get any data from
	; the kernel


	mov rax, 0
	mov rbx, 0
	mov r8, 0
	mov r9, 0
	mov r10, 0
	mov r11, 0
	mov r12, 0
	mov r13, 0
	mov r14, 0
	mov r15, 0
	; TODO XMM registers

	

    ; Switch to user mode by performing a far return
    iretq

ReturnToKernel:

	mov rbp, [NextTask + 0]
	mov rsp, [NextTask + 8]

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
