section .data

%macro PushGeneralPurposeRegisters 0
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
%endmacro


%macro ISR_NO_ERROR 2
    extern ISR_Int_%2
    global ISR_%2
	global DebugHook_ISR_%2
ISR_%2:
	nop
DebugHook_ISR_%2
    PushGeneralPurposeRegisters

    ; Get the value of CR2 (where the faulting instruction accessed)
    mov rax, cr2

    mov rdi, [rsp + 14*8] ; RIP from EIP
    mov rsi, cr2
    xor rdx, rdx
	mov rcx, $%1

    ; Call the function named ISR_Int_ followed by the given ISR name
    call ISR_Int_%2

    PopGeneralPurposeRegisters

    ; Return from the interrupt
    iretq
%endmacro

%macro ISR_WITH_ERROR 2
    extern ISR_Int_%2
    global ISR_%2
	global DebugHook_ISR_%2
ISR_%2:
	; Bodge to get GDB to see the callstack correctly
	add rsp, 8 
DebugHook_ISR_%2:
	sub rsp, 8

    PushGeneralPurposeRegisters

    mov rdi, [rsp + 15*8] ; RIP from EIP
    mov rsi, cr2
    mov rdx, [rsp + 14*8] ; Error code
	mov rcx, $%1
   
    ; Call the function named ISR_Int_ followed by the given ISR name
    call ISR_Int_%2

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
; Coprocessor Segment Overrun is not used
ISR_WITH_ERROR  10, InvalidTaskStateSegment         ; 10/0xA
ISR_WITH_ERROR  11, SegmentNotPresent               ; 11/0xB
ISR_WITH_ERROR  12, StackSegmentFault               ; 12/0xC
ISR_WITH_ERROR  13, GeneralProtectionFault          ; 13/0xD
ISR_WITH_ERROR  14, PageFault                       ; 14/0xE
; 15 is reserved
ISR_NO_ERROR    16, x87FloatingPointException       ; 16
ISR_WITH_ERROR  17, AlignmentCheck                  ; 17
ISR_NO_ERROR    18, MachineCheck                    ; 18
ISR_NO_ERROR    19, SIMDFloatingPointException      ; 19
ISR_NO_ERROR    20, VirtualizationException         ; 20
ISR_WITH_ERROR  21, ControlProtectionException      ; 21
; 22-31 Intel reserved

ISR_NO_ERROR 32, PITInterrupt
ISR_NO_ERROR 33, Unused33
ISR_NO_ERROR 34, Unused34
ISR_NO_ERROR 35, Unused35
ISR_NO_ERROR 36, Unused36
ISR_NO_ERROR 37, Unused37
ISR_NO_ERROR 38, Unused38
ISR_NO_ERROR 39, Unused39
ISR_NO_ERROR 40, Unused40
ISR_NO_ERROR 41, Unused41
ISR_NO_ERROR 42, Unused42
ISR_NO_ERROR 43, Unused43
ISR_NO_ERROR 44, Unused44
ISR_NO_ERROR 45, Unused45
ISR_NO_ERROR 46, Unused46
ISR_NO_ERROR 47, Unused47
ISR_NO_ERROR 48, Unused48
ISR_NO_ERROR 49, Unused49
ISR_NO_ERROR 50, Unused50
ISR_NO_ERROR 51, Unused51
ISR_NO_ERROR 52, Unused52
ISR_NO_ERROR 53, Unused53
ISR_NO_ERROR 54, Unused54
ISR_NO_ERROR 55, Unused55
ISR_NO_ERROR 56, Unused56
ISR_NO_ERROR 57, Unused57
ISR_NO_ERROR 58, Unused58
ISR_NO_ERROR 59, Unused59
ISR_NO_ERROR 60, Unused60
ISR_NO_ERROR 61, Unused61
ISR_NO_ERROR 62, Unused62
ISR_NO_ERROR 63, Unused63
ISR_NO_ERROR 64, Unused64
ISR_NO_ERROR 65, Unused65
ISR_NO_ERROR 66, Unused66
ISR_NO_ERROR 67, Unused67
ISR_NO_ERROR 68, Unused68
ISR_NO_ERROR 69, Unused69
