section .text
global Write
global Exit

%define Syscall_Write 1
%define Syscall_Exit 60

Write:
    ; rdi = fd, rsi = buf, rdx = count
	push rbx
	push r13
    mov rax, Syscall_Write
	mov r13, rsp ; Save stack pointer
	mov rbx, rbp ; Save stack base, rcx contains rsp after the syscall
    syscall
	pop r13
	pop rbx
	ret

Exit:
    ; rdi = exit code
	push rbx
	push r13
    mov rax, Syscall_Exit
	mov r13, rsp ; Save stack pointer
	mov rbx, rbp ; Save stack base, rcx contains RIP after the syscall
    syscall
	pop r13
	pop rbx
	ret
