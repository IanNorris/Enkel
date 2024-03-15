section .text

global GetGSBase

GetGSBase:
    mov rax, gs
    ret
