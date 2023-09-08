section .data

section .text

global APEntry
global APEntryEnd
extern APEntryFunction
align 4

APEntry:
	; rdi cotains PML4
	; rdx contains GDTRegister
	; rcx contains APEntryFunction
	; r8 contains IDTLimit

	;Prepare a stack frame
	mov rax, [rel APEntry]
	push rax

	cli ; Disable interrupts
	mov cr3, rdi
	lgdt [rdx]
	lidt [r8]
	call ReloadSegmentsAP
	sti ; Enable interrupts
	jmp [rcx] ;Jump to APEntryFunction
    hlt

ReloadSegmentsAP:
    push 0x08 ; CS register offset
    lea rax, [rel ReloadAP]
    push rax
    retfq ; This jumps to Reload

ReloadAP: mov ax, 0x10 ; DS register offset
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ret

APEntryEnd:
	hlt
