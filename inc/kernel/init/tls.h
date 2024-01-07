#pragma once

#include "common/types.h"

// See task_struct for offsets that GCC might refer to
struct TLSData
{
	uint64_t Self;
	uint8_t Reserved[0x20];
	uint64_t StackCanary;
} PACKED_ALIGNMENT;

struct TLSAllocation
{
	uint64_t Allocation;
	uint64_t Size;
	uint64_t FSBase;
};

extern "C" KERNEL_API void SetTLSBase(void* TlsBase);

void CreateTLS(TLSAllocation* allocationOut, bool kernel, uint64_t tdataSize, uint64_t tbssSize, uint8_t* tdataStart, uint64_t tlsAlign);
void InitializeKernelTLS();
TLSAllocation* CreateUserModeTLS(uint64_t tdataSize, uint64_t tbssSize, uint8_t* tdataStart, uint64_t tlsAlign);
void DestroyTLS(TLSAllocation* allocation);

uint64_t GetFSBase();
void SetFSBase(uint64_t fsBase);
