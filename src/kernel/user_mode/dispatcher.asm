section .text
global SyscallDispatcher

%macro PushGeneralPurposeRegisters 0
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
%endmacro

%macro PopGeneralPurposeRegisters 0
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
%endmacro

extern SyscallTable

SyscallDispatcher:
	PushGeneralPurposeRegisters

	lea r12, [SyscallTable + rax*8]
	call [r12]
		
	PopGeneralPurposeRegisters

	o64 sysret

	; Ensure clean break after sysretq for visibility
	nop
	nop
	nop
	nop

	; Shouldn't get here
	hlt
