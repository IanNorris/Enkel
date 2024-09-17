section .text
global SyscallDispatcher

extern LoadFS
extern sys_not_implemented

SyscallDispatcher:
	
	push rcx ;RIP

	; Switches to the kernel GS
	swapgs

	call LoadFS

	mov rcx, rsp
	mov rsp, [gs:8]

	; Ensure stack is aligned
	and rsp, ~0xF

	push rcx
	push r11 ; RFLAGS
	push rbp

	mov rbp, rsp
	
	; rax contains our syscall number

    push rdx
    push rsi
    push rdi
    push r8
    push r9
	push r11
    push r12
    push r14
    push r15

	; Prepare the ABI for Sys-V and make registers align with calling convention
	mov rcx, r10

	cmp rax, 400
	jb .valid

	mov r12, sys_not_implemented
	jmp .dispatch

.valid:
	mov r12, [gs:16]
	lea r12, [r12 + rax*8]

.dispatch:
	call [r12]
	
	pop r15
	pop r14
	pop r12
	pop r11
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rdx

	pop rbp
	popfq ; Restore RFLAGS
	pop rsp

	; Switch to user GS
	swapgs

	; Reload FSBase
	call LoadFS

	pop rcx
	
	o64 sysret
