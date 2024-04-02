section .text

global GetGS

GetGS:
    push rcx
    push rdx

    mov ecx, 0xC0000101 ;MSR_GS_BASE
    rdmsr
    
    ; edx:eax now contains the value read from the MSR
    ; Combine edx and eax into rax for 64-bit result
    shl rdx, 32
    or rax, rdx

    pop rdx
    pop rcx

    ret
