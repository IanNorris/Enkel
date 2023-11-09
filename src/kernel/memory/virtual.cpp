#include "memory/physical.h"
#include "kernel/memory/state.h"
#include "kernel/memory/pml4.h"
#include "utilities/termination.h"

extern MemoryState PhysicalMemoryState;
extern MemoryState VirtualMemoryState;

void* VirtualAlloc(uint64_t ByteSize, PrivilegeLevel privilegeLevel)
{
	//TODO: We don't actually need a contiguous block of physical for this!
    uint64_t PhysicalAddress = PhysicalMemoryState.FindMinimumSizeFreeBlock(ByteSize);

	if(PhysicalAddress == 0)
	{
		_ASSERTF(PhysicalAddress == 0, "Failed to allocate");
		return 0;
	}

    uint64_t VirtualAddress = VirtualMemoryState.FindMinimumSizeFreeBlock(ByteSize);

    MapPages(VirtualAddress, PhysicalAddress, ByteSize, true, false, privilegeLevel, MemoryState::RangeState::Used);

	//This should always succeed as we're setting the page as writable
	*(volatile uint64_t*)VirtualAddress = 0x0;
	_ASSERTF(*(volatile uint64_t*)VirtualAddress == 0x0, "Write to address failed");

    return (void*)VirtualAddress;
}

bool VirtualFree(void* Address, uint64_t ByteSize)
{
    uint64_t PhysicalAddress = GetPhysicalAddress((uint64_t)Address);
    uint64_t VirtualAddress = (uint64_t)Address;

    MapPages(VirtualAddress, PhysicalAddress, ByteSize, false, false, PrivilegeLevel::Kernel, MemoryState::RangeState::Free);

	return true;
}

void VirtualProtect(void* Address, uint64_t ByteSize, MemoryProtection ProtectFlags, PageFlags pageFlags)
{
	uint64_t PhysicalAddress = GetPhysicalAddress((uint64_t)Address);
    uint64_t VirtualAddress = (uint64_t)Address;

	bool writable = false;
	bool executable = false;

	switch(ProtectFlags)
	{
		case MemoryProtection::ReadOnly:
			break;

		case MemoryProtection::ReadWrite:
			writable = true;
			break;

		case MemoryProtection::Execute:
			executable = true;
	}

    MapPages(VirtualAddress, PhysicalAddress, ByteSize, writable, executable, PrivilegeLevel::Keep, MemoryState::RangeState::Used, pageFlags);
}
