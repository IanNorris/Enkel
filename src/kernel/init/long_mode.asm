section .text
global CheckProtectedMode:
global LoadPageMapLevel4
global GetCR3

CheckProtectedMode:
	mov rax, cr0
	test al, 1		; LSB contains protected mode flag
	setnz al
	ret

LoadPageMapLevel4:
	mov rax, rdi
	mov cr3, rax
	ret

GetCR3:
	mov rax, cr3
	ret