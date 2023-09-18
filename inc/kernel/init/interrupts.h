#pragma once

void DisableInterrupts();
void EnableInterrupts();
size_t InitIDT(uint8_t* IDT, unsigned int codeSelector);
bool IsDebuggerPresent();
void __attribute__((used, noinline)) OnKernelMainHook();
void __attribute__((used, noinline)) DebuggerHook();

#define DEFINE_NAMED_INTERRUPT(functionName) extern "C" void KERNEL_API __attribute__((used,noinline)) ISR_Int_##functionName
