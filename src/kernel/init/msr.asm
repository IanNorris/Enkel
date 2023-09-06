section .text

global GetMSR
global SetMSR

; uint64_t GetMSR(uint32_t msr);
GetMSR:
    ; rdi = msr

    ; Move rdi (msr number) to ecx for the rdmsr instruction
    mov ecx, edi

    ; Read the MSR value
    rdmsr

    ; edx:eax now contains the value read from the MSR
    ; Combine edx and eax into rax for 64-bit result
    shl rdx, 32
    or rax, rdx

    ret

; void SetMSR(uint32_t msr, uint64_t value);
SetMSR:
    ; rdi = msr, rsi = value (64-bit)

    ; Move rdi (msr number) to ecx for the wrmsr instruction
    mov ecx, edi

    ; Split rsi into edx:eax for high:low 32-bit values
    mov eax, esi            ; Lower 32-bits
    shr rsi, 32
    mov edx, esi            ; Upper 32-bits

    ; Write the values to the MSR
    wrmsr

    ret
