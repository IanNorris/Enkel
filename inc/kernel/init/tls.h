#pragma once

#include "common/types.h"
#include "kernel/init/segments.h"

// See task_struct for offsets that GCC might refer to
struct TLSData
{
	uint64_t Self;
	uint64_t Reserved0;
	uint64_t Reserved1;
	uint64_t Reserved2;
	uint64_t Reserved3;
	uint64_t StackCanary;
};

struct TLSAllocation
{
	TLSData* Memory;
	uint64_t Size;
	uint64_t FSBase;
};

extern "C" KERNEL_API void SetTLSBase(void* TlsBase);

void CreateTLS(TLSAllocation* allocationOut, bool kernel, uint64_t tdataSize, uint64_t tbssSize, uint8_t* tdataStart, uint64_t tlsAlign);
void InitializeKernelTLS();
TLSAllocation* CreateUserModeTLS(uint64_t tdataSize, uint64_t tbssSize, uint8_t* tdataStart, uint64_t tlsAlign);
void DestroyTLS(TLSAllocation* allocation);

void SetUserFSBase(uint64_t fsBase);
void SetUserGS();
uint64_t GetUserFSBase();

uint64_t GetFSBase();
void SetFSBase(uint64_t fsBase);

uint64_t GetGSBase();
void SetKernelGSBase(uint64_t gsBase);
void SetUserGSBase(uint64_t gsBase);
