section .text

; Expects:
; RDI = address of boot data to pass to the kernel
; RSI = address of the kernel function to call
; RDX = new stack pointer

global EnterKernel

EnterKernel:
    ; Set up the new stack pointer
    mov rsp, rdx
    
    ; Clear RBP for the new stack frame
    mov rbp, rsp
    
    ; Call kernel main, rdi is already the boot data
    call rsi
    
    ; Should never get here
    hlt
