section .text

%macro DefineSyscall 2
	global %1
%1:
	push rcx
	push r11
	push rbx
	push r13
    mov rax, %2
	mov r13, rsp ; Save stack pointer
	mov rbx, rbp ; Save stack base, rcx contains rsp after the syscall
    syscall
	pop r13
	pop rbx
	pop r11
	pop rcx
	ret
%endmacro

DefineSyscall Write, 1
DefineSyscall Exit, 60
DefineSyscall InvSysCall, 184