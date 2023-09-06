#pragma once

#include "common/types.h"

extern "C" KERNEL_API uint64_t GetMSR(uint32_t msr);
extern "C" KERNEL_API void SetMSR(uint32_t msr, uint64_t value);
