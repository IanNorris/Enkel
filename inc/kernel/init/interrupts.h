#pragma once

void DisableInterrupts();
void EnableInterrupts();
size_t InitIDT(uint8_t* IDT, unsigned int codeSelector);
bool IsDebuggerPresent();
void __attribute__((used, noinline)) OnKernelMainHook();
void __attribute__((used, noinline)) DebuggerHook();

typedef uint32_t(*InterruptWithContext)(void* Context);

void SetInterruptHandler(int InterruptNumber, InterruptWithContext Handler, void* Context);
void ClearInterruptHandler(int InterruptNumber);

#define DEFINE_NAMED_INTERRUPT(functionName) extern "C" void KERNEL_API __attribute__((used,noinline)) ISR_Int_##functionName
