global SetTLSBase

section .text

SetTLSBase:
    ; wrfsbase rdi - hangs qemu
	mov rax, rdi
	mov rcx, 0xC0000100 ;FSBase 
    wrmsr

	mov rax, rdi
	mov rcx, 0xC0000101 ;GSBase 
    wrmsr

	mov rax, rdi
	mov rcx, 0xC0000102 ;KernelGSBase 
    wrmsr

    ret
