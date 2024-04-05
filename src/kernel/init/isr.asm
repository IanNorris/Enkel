section .data

extern LoadFS
extern GetGS
extern GKernelEnvironment

%macro PushGeneralPurposeRegisters 0
    ; Red zone
    sub rsp, 128

    push rbp
    pushfq
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
%endmacro

%macro PopGeneralPurposeRegisters 0
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
    popfq
    pop rbp

    ; Red zone
    add rsp, 128
%endmacro

maybe_swapgs:
    push rax
    call GetGS
    cmp rax, [GKernelEnvironment]

    je skip_gs

    swapgs
    call LoadFS

skip_gs:

    pop rax

    ret


%macro ISR_NO_ERROR 2
    extern ISR_Int_%2
    global ISR_%2
	global DebugHook_ISR_%2
ISR_%2:
	nop
DebugHook_ISR_%2:

    PushGeneralPurposeRegisters

    ; Clear direction flag
    cld

    mov rdi, %1							    ; Interrupt number
    mov rsi, [rsp + (16*8) + (0*8) + 128] 	; RIP from EIP (param 1)
	mov rdx, cr2 						    ; CR2 (param 2)
	mov rcx, 0						 	    ; Error code (param 3)
	mov r8,  [rsp + (16*8) + (1*8) + 128] 	; CS 
	mov r9,  rbp					    	; RBP

	call maybe_swapgs
    
    ; Call the function named ISR_Int_ followed by the given ISR name
    call ISR_Int_%2

	call maybe_swapgs

    PopGeneralPurposeRegisters

    ; Return from the interrupt
    iretq
%endmacro

%macro ISR_WITH_ERROR 2
    extern ISR_Int_%2
    global ISR_%2
	global DebugHook_ISR_%2
ISR_%2:

	sub rsp, 32
DebugHook_ISR_%2:
	add rsp, 32

	; push 16 regs
    PushGeneralPurposeRegisters

    ; Clear direction flag
    cld

	mov rdi, %1							    ; Interrupt number
    mov rsi, [rsp + (16*8) + (1*8) + 128] 	; RIP from EIP (param 1)
	mov rdx, cr2 						    ; CR2 (param 2)
	mov rcx, [rsp + (16*8) + (0*8) + 128] 	; Error code (param 3)
	mov r8,  [rsp + (16*8) + (2*8) + 128]	; CS 
	mov r9,  rbp					 	    ; RBP

	call maybe_swapgs
   
    ; Call the function named ISR_Int_ followed by the given ISR name
    call ISR_Int_%2

	call maybe_swapgs

    PopGeneralPurposeRegisters

    

    ; Return from the interrupt
    iretq
%endmacro

ISR_NO_ERROR     0, DivideByZero                    ; 0
ISR_NO_ERROR     1, SingleStep                      ; 1
ISR_NO_ERROR     2, NonMaskableInterrupt            ; 2
ISR_NO_ERROR     3, Breakpoint                      ; 3
ISR_NO_ERROR     4, Overflow                        ; 4
ISR_NO_ERROR     5, BoundRangeExceeded              ; 5
ISR_NO_ERROR     6, InvalidOpcode                   ; 6
ISR_NO_ERROR     7, CoprocessorUnavailable          ; 7
ISR_WITH_ERROR   8, DoubleFault                     ; 8
ISR_NO_ERROR 	 9, Callback9						; 9
ISR_WITH_ERROR  10, InvalidTaskStateSegment         ; 10/0xA
ISR_WITH_ERROR  11, SegmentNotPresent               ; 11/0xB
ISR_WITH_ERROR  12, StackSegmentFault               ; 12/0xC
ISR_WITH_ERROR  13, GeneralProtectionFault          ; 13/0xD
ISR_WITH_ERROR  14, PageFault                       ; 14/0xE
ISR_NO_ERROR    15, Callback15						; 15/0xF
ISR_NO_ERROR    16, x87FloatingPointException       ; 16
ISR_WITH_ERROR  17, AlignmentCheck                  ; 17
ISR_NO_ERROR    18, MachineCheck                    ; 18
ISR_NO_ERROR    19, SIMDFloatingPointException      ; 19
ISR_NO_ERROR    20, VirtualizationException         ; 20
ISR_WITH_ERROR  21, ControlProtectionException      ; 21
ISR_NO_ERROR 	22, Callback22						; 22
ISR_NO_ERROR 	23, Callback23						; 23
ISR_NO_ERROR 	24, Callback24						; 24
ISR_NO_ERROR 	25, Callback25						; 25
ISR_NO_ERROR 	26, Callback26						; 26
ISR_NO_ERROR 	27, Callback27						; 27
ISR_NO_ERROR 	28, Callback28						; 28
ISR_NO_ERROR 	29, Callback29						; 29
ISR_NO_ERROR 	30, Callback30						; 30
ISR_NO_ERROR 	31, Callback31						; 31
ISR_NO_ERROR 	32, PITInterrupt
ISR_NO_ERROR 	33, Callback33
ISR_NO_ERROR 	34, Callback34
ISR_NO_ERROR 	35, Callback35
ISR_NO_ERROR 	36, Callback36
ISR_NO_ERROR 	37, Callback37
ISR_NO_ERROR 	38, Callback38
ISR_NO_ERROR 	39, Callback39
ISR_NO_ERROR 	40, Callback40
ISR_NO_ERROR 	41, Callback41
ISR_NO_ERROR 	42, Callback42
ISR_NO_ERROR 	43, Callback43
ISR_NO_ERROR 	44, Callback44
ISR_NO_ERROR 	45, Callback45
ISR_NO_ERROR 	46, Callback46
ISR_NO_ERROR 	47, Callback47
ISR_NO_ERROR 	48, Callback48
ISR_NO_ERROR 	49, Callback49
ISR_NO_ERROR 	50, Callback50
ISR_NO_ERROR 	51, Callback51
ISR_NO_ERROR 	52, Callback52
ISR_NO_ERROR 	53, Callback53
ISR_NO_ERROR 	54, Callback54
ISR_NO_ERROR 	55, Callback55
ISR_NO_ERROR 	56, Callback56
ISR_NO_ERROR 	57, Callback57
ISR_NO_ERROR 	58, Callback58
ISR_NO_ERROR 	59, Callback59
ISR_NO_ERROR 	60, Callback60
ISR_NO_ERROR 	61, Callback61
ISR_NO_ERROR 	62, Callback62
ISR_NO_ERROR 	63, Callback63
ISR_NO_ERROR 	64, Callback64
ISR_NO_ERROR 	65, Callback65
ISR_NO_ERROR 	66, Callback66
ISR_NO_ERROR 	67, Callback67
ISR_NO_ERROR 	68, Callback68
ISR_NO_ERROR 	69, Callback69

ISR_NO_ERROR 255, SpuriousInterrupt
