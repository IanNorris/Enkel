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
#define MSR_GS_BASE_USER 0xC0000101
#define MSR_GS_BASE_KERNEL 0xC0000102


EnvironmentKernel* GKernelEnvironment = nullptr;
EnvironmentUser* GUserEnvironment = nullptr;

void SetUserFSBase(uint64_t fsBase)
{
	GUserEnvironment->FSBase = fsBase;
}

void SetUserGS()
{
	SetUserGSBase((uint64_t)GUserEnvironment);
}

uint64_t GetUserFSBase()
{
	return GUserEnvironment->FSBase;
}

uint64_t GetFSBase()
{
	return GetMSR(MSR_FS_BASE);
}

void SetFSBase(uint64_t fsBase)
{
	SetMSR(MSR_FS_BASE, fsBase);
}

void SetKernelGSBase(uint64_t gsBase)
{
	//Reversed as we do swapgs before first use
	SetMSR(MSR_GS_BASE_USER, gsBase);
}

void SetUserGSBase(uint64_t gsBase)
{
	//Reversed as we do swapgs before first use
	SetMSR(MSR_GS_BASE_KERNEL, gsBase);
}

void CreateTLS(TLSAllocation* allocationOut, bool kernel, uint64_t tdataSize, uint64_t tbssSize, uint8_t* tdataStart, uint64_t tlsAlign)
{
	memset(allocationOut, 0, sizeof(TLSAllocation));

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
	kernelData.Self = (uint64_t)tls + alignedSize;
	kernelData.Reserved0 = 0x715000AA;
	kernelData.Reserved1 = 0x715000BB;
	kernelData.Reserved2 = 0x715000CC;
	kernelData.Reserved3 = 0x715000DD;
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

	allocationOut->Memory = (TLSData*)tls;
	allocationOut->Size = tlsAllocationSize;
	allocationOut->FSBase = tlsHigh;
}

//TODO make this core specific
void InitializeKernelTLS()
{ 
    size_t tdata_size = (uint8_t*)&__tdata_end - (uint8_t*)&__tdata_start;
    size_t tbss_size = (uint8_t*)&__tbss_end - (uint8_t*)&__tbss_start;
    size_t tls_size = tdata_size + tbss_size;

	TLSAllocation allocation;

	CreateTLS(&allocation, true, tdata_size, tbss_size, (uint8_t*)&__tdata_start, 0x1);

	SetFSBase(allocation.FSBase);

	GKernelEnvironment = (EnvironmentKernel*)VirtualAlloc(4096, PrivilegeLevel::Kernel);
	GUserEnvironment = (EnvironmentUser*)VirtualAlloc(4096, PrivilegeLevel::User);

	GKernelEnvironment->FSBase = allocation.FSBase;

	SetKernelGSBase((uint64_t)GKernelEnvironment);
}

TLSAllocation* CreateUserModeTLS(uint64_t tdataSize, uint64_t tbssSize, uint8_t* tdataStart, uint64_t tlsAlign)
{
	TLSAllocation* allocation = (TLSAllocation*)rpmalloc(sizeof(TLSAllocation));

	CreateTLS(allocation, false, tdataSize, tbssSize, tdataStart, tlsAlign);

	return allocation;
}

void DestroyTLS(TLSAllocation* allocation)
{
	VirtualFree((void*)allocation->Memory, allocation->Size);
	rpfree(allocation);
}
