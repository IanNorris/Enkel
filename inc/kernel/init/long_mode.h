#pragma once

extern "C" int KERNEL_API CheckProtectedMode();
extern "C" void KERNEL_API LoadPageMapLevel4(uint64_t);
extern "C" uint64_t KERNEL_API GetCR0();
extern "C" uint64_t KERNEL_API GetCR1();
extern "C" uint64_t KERNEL_API GetCR2();
extern "C" uint64_t KERNEL_API GetCR3();
extern "C" uint64_t KERNEL_API GetCR4();

extern "C" void KERNEL_API SetCR0(uint64_t value);
extern "C" void KERNEL_API SetCR1(uint64_t value);
extern "C" void KERNEL_API SetCR2(uint64_t value);
extern "C" void KERNEL_API SetCR3(uint64_t value);
extern "C" void KERNEL_API SetCR4(uint64_t value);
