#include "common/types.h"
#include "kernel/init/long_mode.h"

void InitCpuExtensions()
{
    //Init SSE3
	uint64_t cr0 = GetCR0();
	cr0 &= ~(1 << 2); // Clear EM
	cr0 |= (1 << 1) | (1 << 5); //Set MP and NE
	SetCR0(cr0);

	uint64_t cr4 = GetCR4();
	cr4 |= (1 << 10); //Set OSXMMEXCPT
	cr4 |= (1 << 9); //Set OSXSAVE
	SetCR4(cr4);
}