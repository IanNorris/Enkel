section .text
global CheckProtectedMode:
global LoadPageMapLevel4
global GetCR0
global GetCR1
global GetCR2
global GetCR3
global GetCR4
global SetCR0
global SetCR1
global SetCR2
global SetCR3
global SetCR4

CheckProtectedMode:
	mov rax, cr0
	test al, 1		; LSB contains protected mode flag
	setnz al
	ret

LoadPageMapLevel4:
	mov rax, rdi
	mov cr3, rax
	ret

GetCR0:
	mov rax, cr0
	ret

GetCR1:
	mov rax, cr1
	ret

GetCR2:
	mov rax, cr2
	ret

GetCR3:
	mov rax, cr3
	ret

GetCR4:
	mov rax, cr4
	ret

SetCR0:
	mov cr0, rdi
	ret

SetCR1:
	mov cr1, rdi
	ret

SetCR2:
	mov cr2, rdi
	ret

SetCR3:
	mov cr3, rdi
	ret

SetCR4:
	mov cr4, rdi
	ret