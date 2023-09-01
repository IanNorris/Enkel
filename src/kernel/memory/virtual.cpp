#include "memory/virtual.h"
#include "kernel/memory/physical.h"
#include "kernel/memory/pml4.h"
#include "utilities/termination.h"

extern SPhysicalState* PhysicalStateRoot;

extern void MapPages(uint64_t virtualAddress, uint64_t physicalAddress, uint64_t size, bool writable, bool executable, bool debug, EPhysicalState newState);

void* VirtualAlloc(uint64_t ByteSize)
{
    uint64_t PhysicalAddress = FindMinimumSizeFreeBlock(PhysicalStateRoot, ByteSize);

	if(PhysicalAddress == 0)
	{
		_ASSERTF(PhysicalAddress == 0, "Failed to allocate");
		return 0;
	}

    uint64_t VirtualAddress = 0x85000000;

    MapPages(VirtualAddress, PhysicalAddress, ByteSize, true, false, false, EPhysicalState::Used);

    return (void*)VirtualAddress;
}

void VirtualFree(void* Address, uint64_t ByteSize)
{
    uint64_t PhysicalAddress = GetPhysicalAddress((uint64_t)Address);
    uint64_t VirtualAddress = (uint64_t)Address;

    MapPages(VirtualAddress, PhysicalAddress, ByteSize, false, false, false, EPhysicalState::Free);
}

void VirtualProtect(void* Address, uint64_t ByteSize, EMemoryProtection ProtectFlags)
{

}
