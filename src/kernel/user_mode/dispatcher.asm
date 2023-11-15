section .text
global SyscallDispatcher

extern SyscallTable

SyscallDispatcher:
	push rcx ; Contains RIP
	push r13 ; Contains the originating stack pointer
	push rbx ; Our UM syscall invoke should put rbp in rbx
	push r12

	lea r12, [SyscallTable + rax*8]
	call [r12]
		
	pop r12
	pop rbx
	pop r13
	pop rcx
	mov rbp, rbx
	mov rsp, r13

	o64 sysret

	; Ensure clean break after sysretq for visibility
	nop
	nop
	nop
	nop

	; Shouldn't get here
	hlt
