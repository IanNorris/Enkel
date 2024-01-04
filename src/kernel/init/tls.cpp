#include "common/types.h"
#include "kernel/init/tls.h"
#include "kernel/init/msr.h"
#include "memory/memory.h"
#include "kernel/memory/state.h"
#include "memory/virtual.h"
#include "rpmalloc.h"

extern "C" void* __tdata_start;
extern "C" void* __tdata_end;
extern "C" void* __tbss_start;
extern "C" void* __tbss_end;

#define MSR_FS_BASE 0xC0000100

uint64_t GetFSBase()
{
	return GetMSR(MSR_FS_BASE);
}

void SetFSBase(uint64_t fsBase)
{
	SetMSR(MSR_FS_BASE, fsBase);
}

// TODO:
// https://wiki.osdev.org/SWAPGS

void* InitializeTLS(bool kernel, uint64_t tdataSize, uint64_t tbssSize, uint8_t* tdataStart, uint64_t tlsAlign)
{
	//https://stackoverflow.com/questions/67343020/understanding-elf-tbss-and-tdata-section-loading

	//Payload looks like this:
	// <tdata> <tbss> <TLSData>
	//               ^- TLS HIGH

	tdataSize = AlignSize(tdataSize, tlsAlign);
	tbssSize = AlignSize(tbssSize, tlsAlign);

	if(tbssSize == 0 && tdataSize == 0)
	{
		tbssSize = PAGE_SIZE;
	}

	uint64_t tlsSize = tdataSize + tbssSize;

	uint64_t alignedSize = AlignSize(tlsSize, PAGE_SIZE);
	uint64_t alignedOffset = alignedSize - tlsSize;

	uint64_t tlsAllocationSize = alignedSize + PAGE_SIZE;

    // Allocate memory for the TLS.
    uint8_t* tls = (uint8_t*)VirtualAlloc(tlsAllocationSize, kernel ? PrivilegeLevel::Kernel : PrivilegeLevel::User);
	memset(tls, 0xFE, tlsAllocationSize);

	TLSData kernelData;
	memset(&kernelData, 0, sizeof(TLSData));
	kernelData.StackCanary = 0xDEADC0DEDEADC0DE;

	memcpy(tls + alignedSize, &kernelData, sizeof(TLSData));

    // Initialize the .tdata section.
	if(tdataSize)
	{
	    memcpy(tls + alignedOffset, tdataStart, tdataSize);
	}

    // Initialize the .tbss section to zero.
    memset(tls + alignedOffset + tdataSize, 0x0, tbssSize);

	uint64_t tlsHigh = (uint64_t)tls + alignedSize;
	SetFSBase(tlsHigh);

	//TODO: LEAKY LEAKY!
	return (void*)tlsHigh;
}

void* InitializeKernelTLS()
{ 
    size_t tdata_size = (uint8_t*)&__tdata_end - (uint8_t*)&__tdata_start;
    size_t tbss_size = (uint8_t*)&__tbss_end - (uint8_t*)&__tbss_start;
    size_t tls_size = tdata_size + tbss_size;

	return InitializeTLS(true, tdata_size, tbss_size, (uint8_t*)&__tdata_start, 0x1);
}

void* InitializeUserModeTLS(uint64_t tdataSize, uint64_t tbssSize, uint8_t* tdataStart, uint64_t tlsAlign)
{
	return InitializeTLS(false, tdataSize, tbssSize, tdataStart, tlsAlign);
}
