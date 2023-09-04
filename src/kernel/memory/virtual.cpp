#include "memory/virtual.h"
#include "kernel/memory/state.h"
#include "kernel/memory/pml4.h"
#include "utilities/termination.h"

extern MemoryState PhysicalMemoryState;
extern MemoryState VirtualMemoryState;

extern void MapPages(uint64_t virtualAddress, uint64_t physicalAddress, uint64_t size, bool writable, bool executable, MemoryState::RangeState newState);

void* VirtualAlloc(uint64_t ByteSize)
{
	//TODO: We don't actually need a contiguous block of physical for this!
    uint64_t PhysicalAddress = PhysicalMemoryState.FindMinimumSizeFreeBlock(ByteSize);

	if(PhysicalAddress == 0)
	{
		_ASSERTF(PhysicalAddress == 0, "Failed to allocate");
		return 0;
	}

    uint64_t VirtualAddress = VirtualMemoryState.FindMinimumSizeFreeBlock(ByteSize);

    MapPages(VirtualAddress, PhysicalAddress, ByteSize, true, false, MemoryState::RangeState::Used);

    return (void*)VirtualAddress;
}

void VirtualFree(void* Address, uint64_t ByteSize)
{
    uint64_t PhysicalAddress = GetPhysicalAddress((uint64_t)Address);
    uint64_t VirtualAddress = (uint64_t)Address;

    MapPages(VirtualAddress, PhysicalAddress, ByteSize, false, false, MemoryState::RangeState::Free);
}

void VirtualProtect(void* Address, uint64_t ByteSize, MemoryProtection ProtectFlags)
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

    MapPages(VirtualAddress, PhysicalAddress, ByteSize, writable, executable, MemoryState::RangeState::Used);
}
