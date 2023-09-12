section .text

global GetCurrentStackFrame

GetCurrentStackFrame:
	mov rax, rbp
	ret
