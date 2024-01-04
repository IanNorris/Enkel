#pragma once

#include "common/types.h"

// See task_struct for offsets that GCC might refer to
struct TLSData
{
	uint8_t Reserved[0x28];
	uint64_t StackCanary;
};

extern "C" KERNEL_API void SetTLSBase(void* TlsBase);

void* InitializeKernelTLS();
void* InitializeUserModeTLS(uint64_t tdataSize, uint64_t tbssSize, uint8_t* tdataStart, uint64_t tlsAlign);

uint64_t GetFSBase();
void SetFSBase(uint64_t fsBase);
