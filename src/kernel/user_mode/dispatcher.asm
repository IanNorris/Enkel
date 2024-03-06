section .text
global SyscallDispatcher

extern SyscallStack
extern SyscallTable
extern sys_not_implemented

SyscallDispatcher:
	mov r10, rsp
	mov rsp, [SyscallStack]
	push r10
	push rbp
	mov rbp, [SyscallStack]
	
	; rax contains our syscall number

	push r11 ; RFLAGS
	push r11 ; push twice so we can also restore r11

	push rcx ; RIP
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r12
    push r14
    push r15

	; TODO: Set FSBase, GS

	cmp rax, 200
	jb .valid

	mov r12, [sys_not_implemented]
	jmp .dispatch

.valid:
	lea r12, [SyscallTable + rax*8]

.dispatch:
	call [r12]
	
	pop r15
	pop r14
	pop r12
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rdx
	pop rcx

	pop r11
	popfq ; Restore RFLAGS

	pop rbp
	pop rsp
	
	o64 sysret

	; Ensure clean break after sysretq for visibility
	nop
	nop
	nop
	nop

	; Shouldn't get here
	hlt
