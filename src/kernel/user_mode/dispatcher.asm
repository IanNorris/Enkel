section .text
global SyscallDispatcher

extern LoadFS
extern SyscallStack
extern SyscallTable
extern sys_not_implemented

SyscallDispatcher:
	
	; Switches to the kernel GS
	swapgs

	call LoadFS

	mov r10, rsp
	mov rsp, [SyscallStack]

	; Ensure stack is aligned
	and rsp, 0xFFFFFFFFFFFFFFEF ; ~0x10

	push r10
	push r11 ; RFLAGS
	push rbp

	mov rbp, rsp
	
	; rax contains our syscall number

	push rcx ; RIP
    push rdx
    push rsi
    push rdi
    push r8
    push r9
	push r11
    push r12
    push r14
    push r15

	cmp rax, 400
	jb .valid

	mov r12, sys_not_implemented
	jmp .dispatch

.valid:
	lea r12, [SyscallTable + rax*8]

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
	pop rcx

	pop rbp
	popfq ; Restore RFLAGS
	pop rsp

	; Switch to user GS
	swapgs

	; Reload FSBase
	call LoadFS
	
	o64 sysret

	; Ensure clean break after sysretq for visibility
	nop
	nop
	nop
	nop

	; Shouldn't get here
	hlt
