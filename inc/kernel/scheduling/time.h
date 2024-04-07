#pragma once

#include "common/types.h"

extern "C" KERNEL_API uint64_t _rdtsc();

void HpetSleepNS(uint64_t nanoseconds);
void HpetSleepUS(uint64_t microseconds);
void HpetSleepMS(uint64_t milliseconds);
