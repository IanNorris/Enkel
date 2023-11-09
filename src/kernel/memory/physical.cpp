#include "memory/physical.h"
#include "kernel/memory/state.h"
#include "kernel/memory/pml4.h"
#include "utilities/termination.h"

extern MemoryState PhysicalMemoryState;
extern MemoryState VirtualMemoryState;

void* PhysicalAlloc(uint64_t PhysicalAddress, uint64_t ByteSize, PrivilegeLevel privilegeLevel, PageFlags pageFlags)
{
    uint64_t VirtualAddress = VirtualMemoryState.FindMinimumSizeFreeBlock(ByteSize);

    MapPages(VirtualAddress, PhysicalAddress, ByteSize, true, false, privilegeLevel, MemoryState::RangeState::Used, pageFlags);

    return (void*)VirtualAddress;
}

void* PhysicalAllocLowestAddress(uint64_t ByteSize, PrivilegeLevel privilegeLevel, PageFlags pageFlags)
{
	uint64_t PhysicalAddress = PhysicalMemoryState.FindMinimumSizeFreeBlock(ByteSize);
    uint64_t VirtualAddress = VirtualMemoryState.FindMinimumSizeFreeBlock(ByteSize);

    MapPages(VirtualAddress, PhysicalAddress, ByteSize, true, false, privilegeLevel, MemoryState::RangeState::Used, pageFlags);

    return (void*)VirtualAddress;
}
