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


%macro ISR_NO_ERROR 1
    extern ISR_Int_%1
    global ISR_%1
	global DebugHook_ISR_%1
ISR_%1:
	nop
DebugHook_ISR_%1
    PushGeneralPurposeRegisters

    ; Get the value of CR2 (where the faulting instruction accessed)
    mov rax, cr2

    mov rdi, [rsp + 14*8] ; RIP from EIP
    mov rsi, cr2
    xor rdx, rdx

    ; Call the function named ISR_Int_ followed by the given ISR name
    call ISR_Int_%1

    PopGeneralPurposeRegisters

    ; Return from the interrupt
    iretq
%endmacro

%macro ISR_WITH_ERROR 1
    extern ISR_Int_%1
    global ISR_%1
	global DebugHook_ISR_%1
ISR_%1:
	; Bodge to get GDB to see the callstack correctly
	add rsp, 8 
DebugHook_ISR_%1:
	sub rsp, 8

    PushGeneralPurposeRegisters

    mov rdi, [rsp + 15*8] ; RIP from EIP
    mov rsi, cr2
    mov rdx, [rsp + 14*8] ; Error code
   
    ; Call the function named ISR_Int_ followed by the given ISR name
    call ISR_Int_%1

    PopGeneralPurposeRegisters

    ; Return from the interrupt
    iretq
%endmacro

ISR_NO_ERROR    DivideByZero                    ; 0
ISR_NO_ERROR    SingleStep                      ; 1
ISR_NO_ERROR    NonMaskableInterrupt            ; 2
ISR_NO_ERROR    Breakpoint                      ; 3
ISR_NO_ERROR    Overflow                        ; 4
ISR_NO_ERROR    BoundRangeExceeded              ; 5
ISR_NO_ERROR    InvalidOpcode                   ; 6
ISR_NO_ERROR    CoprocessorUnavailable          ; 7
ISR_WITH_ERROR  DoubleFault                     ; 8
; Coprocessor Segment Overrun is not used
ISR_WITH_ERROR  InvalidTaskStateSegment         ; 10/0xA
ISR_WITH_ERROR  SegmentNotPresent               ; 11/0xB
ISR_WITH_ERROR  StackSegmentFault               ; 12/0xC
ISR_WITH_ERROR  GeneralProtectionFault          ; 13/0xD
ISR_WITH_ERROR  PageFault                       ; 14/0xE
; 15 is reserved
ISR_NO_ERROR    x87FloatingPointException       ; 16
ISR_WITH_ERROR  AlignmentCheck                  ; 17
ISR_NO_ERROR    MachineCheck                    ; 18
ISR_NO_ERROR    SIMDFloatingPointException      ; 19
ISR_NO_ERROR    VirtualizationException         ; 20
ISR_WITH_ERROR  ControlProtectionException      ; 21
; 22-31 Intel reserved

ISR_NO_ERROR Unused32
ISR_NO_ERROR Unused33
ISR_NO_ERROR Unused34
ISR_NO_ERROR Unused35
ISR_NO_ERROR Unused36
ISR_NO_ERROR Unused37
ISR_NO_ERROR Unused38
ISR_NO_ERROR Unused39
ISR_NO_ERROR Unused40
ISR_NO_ERROR Unused41
ISR_NO_ERROR Unused42
ISR_NO_ERROR Unused43
ISR_NO_ERROR Unused44
ISR_NO_ERROR Unused45
ISR_NO_ERROR Unused46
ISR_NO_ERROR Unused47
ISR_NO_ERROR Unused48
ISR_NO_ERROR Unused49
ISR_NO_ERROR Unused50
ISR_NO_ERROR Unused51
ISR_NO_ERROR Unused52
ISR_NO_ERROR Unused53
ISR_NO_ERROR Unused54
ISR_NO_ERROR Unused55
ISR_NO_ERROR Unused56
ISR_NO_ERROR Unused57
ISR_NO_ERROR Unused58
ISR_NO_ERROR Unused59
ISR_NO_ERROR Unused60
ISR_NO_ERROR Unused61
ISR_NO_ERROR Unused62
ISR_NO_ERROR Unused63
ISR_NO_ERROR Unused64
ISR_NO_ERROR Unused65
ISR_NO_ERROR Unused66
ISR_NO_ERROR Unused67
ISR_NO_ERROR Unused68
ISR_NO_ERROR Unused69
