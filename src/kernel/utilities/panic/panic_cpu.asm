section .text

global GetPanicCPUState

; Space for GDTR, IDTR (limit: 16 bits, base: 64 bits)
; Note: Adjust struct offsets as necessary
TempTableRegSpace db 10h dup(0)

GetPanicCPUState:
	; RDI will contain the pointer to the CPURegs struct

	; Save current registers that will be overwritten
	push rax

	; Store general purpose registers
	mov [rdi + 0x00], rax
	mov [rdi + 0x08], rbx
	mov [rdi + 0x10], rcx
	mov [rdi + 0x18], rdx
	mov [rdi + 0x20], rsi
	mov [rdi + 0x28], rdi ; Store original RDI value (struct pointer)

	; R8 - R15 require moving their values without altering them
	mov [rdi + 0x30], r8
	mov [rdi + 0x38], r9
	mov [rdi + 0x40], r10
	mov [rdi + 0x48], r11
	mov [rdi + 0x50], r12
	mov [rdi + 0x58], r13
	mov [rdi + 0x60], r14
	mov [rdi + 0x68], r15

	; Store RIP, RSP, RBP, and RFLAGS (these require special handling)
	; For RIP, you might need to use a call/pop sequence or similar to get the current instruction pointer
	; For RSP, add the size of the stack space we've used in this function
	; RBP can be moved directly
	; RFLAGS can be read with pushfq/pop

	; Example for RIP (not accurate for all contexts, especially if interrupts are involved)
	lea rax, [rel Next] ; Get current RIP (approximation)
Next:
	mov [rdi + 0x70], rax

	; Correct RSP for the current stack usage
	lea rax, [rsp + 0x8] ; Adjust for pushes
	mov [rdi + 0x78], rax

	mov [rdi + 0x80], rbp

	; RFLAGS
	pushfq
	pop rax
	mov [rdi + 0x88], rax

	; CR0, CR2, CR3, CR4 (requires moving from control registers)
	mov rax, cr0
	mov [rdi + 0x90], rax
	mov rax, cr2
	mov [rdi + 0x98], rax
	mov rax, cr3
	mov [rdi + 0xA0], rax
	mov rax, cr4
	mov [rdi + 0xA8], rax

	; Store GDTR
	sgdt [TempTableRegSpace]
	mov rax, [TempTableRegSpace + 2] ; Skip 2-byte limit
	mov [rdi + 0xB0], rax ; Store GDTR base

	; Store LDTR
	sldt ax
	mov [rdi + 0xB8], rax ; Store LDTR (16-bit, but placed in a 64-bit field for alignment)

	; Store IDTR
	sidt [TempTableRegSpace]
	mov rax, [TempTableRegSpace + 2] ; Skip 2-byte limit
	mov [rdi + 0xC0], rax ; Store IDTR base

	; Store TR
	str ax
	mov [rdi + 0xC8], rax ; Store TR (16-bit, but placed in a 64-bit field for alignment)

	; Save registers that will be used
	push rcx
	push rdx

	; Read FS base
	mov ecx, 0xC0000100
	rdmsr
	mov [rdi + 0xD0], eax ; Lower 32 bits of FS base
	mov [rdi + 0xD4], edx ; Upper 32 bits of FS base, if needed

	; Read GS base
	mov ecx, 0xC0000101
	rdmsr
	mov [rdi + 0xD8], rax ; Lower 32 bits of GS base
	mov [rdi + 0xDC], rdx ; Upper 32 bits of GS base, if needed


	; Read GS base kernel
	mov ecx, 0xC0000102
	rdmsr
	mov [rdi + 0xE0], rax ; Lower 32 bits of GS base kernel
	mov [rdi + 0xE4], rdx ; Upper 32 bits of GS base kernel, if needed

	; Restore registers
	pop rdx
	pop rcx

	; Read segment registers
	mov ax, cs
	mov [rdi + 0xE8], ax
	mov ax, ds
	mov [rdi + 0xEA], ax
	mov ax, ss
	mov [rdi + 0xEC], ax
	mov ax, es
	mov [rdi + 0xEE], ax
	mov ax, fs
	mov [rdi + 0xF0], ax
	mov ax, gs
	mov [rdi + 0xF2], ax

	; Restore the registers we pushed at the beginning
	pop rax

	ret
