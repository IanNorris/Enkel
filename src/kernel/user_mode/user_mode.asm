section .text
global SwitchToUserMode
global ReturnToKernel
global LoadFS

LoadFS:
	; Set FSBase from [gs:0]
	push rcx
	push rdx
	push rax
	push rsi

	mov ecx, 0xC0000100 ;FSBase
	mov rsi, [gs:0]
	mov rsi, [rsi]

    ; Split rsi into edx:eax for high:low 32-bit values
    mov eax, esi            ; Lower 32-bits
    shr rsi, 32
    mov edx, esi            ; Upper 32-bits

    ; Write the values to the MSR
    wrmsr

	pop rsi
	pop rax
	pop rdx
	pop rcx
	; End of set FSBase

	ret

SwitchToUserMode:
	;   rdi - uint64_t stackPointer (user mode stack pointer)
	;   rsi - uint64_t entry (user mode entry point address)
	;	rdx - uint16_t userModeCS
	;	rcx - uint16_t userModeDS

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
    pushfq

	; Can't store these on the stack as we need a way to get
	; these back when we ReturnToKernel
	mov [gs:24], rbp ; KernelRBP
	mov [gs:32], rsp ; KernelRSP

    ; Clear the direction flag
    cld

	push 0x08 ; CS register offset
	push rdi

	mov rbp, rdi

	push rcx ;UM DS
	push rdi ;UM stack
	push 0x202 ; rflags (interrupts enabled, reserved always on flag set)
	push rdx ;UM CS
	push rsi ;UM entry

	; Clear all unused registers to 0
	; so the program doesn't get any 
	; leaked data from the kernel

	mov rax, 0
	mov rbx, 0
	mov r8, 0
	mov r9, 0
	mov r10, 0
	mov r11, 0
	mov r12, 0
	mov r13, 0
	mov r14, 0
	mov r15, 0
	; TODO XMM registers

	swapgs

	; Reload FSBase
	call LoadFS

    ; Switch to user mode by performing a far return
    iretq

ReturnToKernel:

	mov rbp, [gs:24]
	mov rsp, [gs:32]

	popfq
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

	ret
