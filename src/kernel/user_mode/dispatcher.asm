section .text
global SyscallDispatcher

extern SyscallStack
extern SyscallTable

SyscallDispatcher:
	; push rax - don't push rax because we're going to stomp it
	; rax contains our syscall number
	push r13 ; RSP
	push rbx ; RBP
	push r11 ; RFLAGS

    push rbx
	push rcx ; RIP
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
	
	mov rbx, rsp

	mov rsp, [SyscallStack]
	mov rbp, [SyscallStack]

	push rbx ; Save the original stack pointer

	; TODO: Set FSBase, GS

	lea r12, [SyscallTable + rax*8]
	call [r12]

	pop rsp ; Pop the original stack pointer (this will get us back our pushes above)
	
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
