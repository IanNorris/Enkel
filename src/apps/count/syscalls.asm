section .text
global Write
global Exit

%macro DefineSyscall 2
	global %1
%1:
	push rbx
	push r13
    mov rax, %2
	mov r13, rsp ; Save stack pointer
	mov rbx, rbp ; Save stack base, rcx contains rsp after the syscall
    syscall
	pop r13
	pop rbx
	ret
%endmacro

DefineSyscall Write, 1
DefineSyscall Exit, 60
