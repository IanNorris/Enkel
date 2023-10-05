#pragma once

#include "kernel/init/bootload.h"

void InitInterrupts(KernelBootData* bootData);
void InitVirtualMemory(KernelBootData* bootData);
void InitRPMalloc();

