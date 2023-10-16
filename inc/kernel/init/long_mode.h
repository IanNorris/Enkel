#pragma once

extern "C" int KERNEL_API CheckProtectedMode();
extern "C" void KERNEL_API LoadPageMapLevel4(uint64_t);
extern "C" uint64_t KERNEL_API GetCR3();
