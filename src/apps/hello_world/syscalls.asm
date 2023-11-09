section .text
global Write
global Exit

%define Syscall_Write 0x71
%define Syscall_Exit 0x92

Write:
    ; rdi = fd, rsi = buf, rdx = count
    mov rax, Syscall_Write
    syscall

Exit:
    ; rdi = exit code
    mov rax, Syscall_Exit
    syscall
