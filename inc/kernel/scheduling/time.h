#pragma once

#include "common/types.h"

extern "C" KERNEL_API uint64_t _rdtsc();

void HpetSleepUS(uint64_t microseconds);
void HpetSleepMS(uint64_t milliseconds);
