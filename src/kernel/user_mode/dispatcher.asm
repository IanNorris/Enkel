section .text
global SyscallDispatcher

extern LoadFS
extern SyscallTable
extern sys_not_implemented

SyscallDispatcher:
	
	push rcx ;RIP

	; Switches to the kernel GS
	swapgs

	call LoadFS

	mov rcx, rsp
	mov rsp, [gs:8]

	; Ensure stack is aligned
	and rsp, 0xFFFFFFFFFFFFFFEF ; ~0x10

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

	pop rbp
	popfq ; Restore RFLAGS
	pop rsp

	; Switch to user GS
	swapgs

	; Reload FSBase
	call LoadFS

	pop rcx
	
	o64 sysret

	; Ensure clean break after sysretq for visibility
	nop
	nop
	nop
	nop

	; Shouldn't get here
	hlt
