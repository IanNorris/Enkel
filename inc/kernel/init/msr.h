#pragma once

#include "common/types.h"

extern "C" KERNEL_API uint64_t GetMSR(uint32_t msr);
extern "C" KERNEL_API void SetMSR(uint32_t msr, uint64_t value);
extern "C" KERNEL_API void OutPort(uint16_t port, uint8_t value);
extern "C" KERNEL_API uint8_t InPort(uint16_t port);
