#include "memory/virtual.h"
#include "kernel/memory/physical.h"

extern void MapPages(uint64_t virtualAddress, uint64_t physicalAddress, uint64_t size, bool writable, bool executable, bool debug, EPhysicalState newState);

void* VirtualAlloc(uint64_t ByteSize)
{
    uint64_t PhysicalAddress = 0x20000000;
    uint64_t VirtualAddress = 0x85000000;

    MapPages(VirtualAddress, PhysicalAddress, ByteSize, true, false, false, EPhysicalState::Used);

    return (void*)VirtualAddress;
}

void VirtualFree(void* Address, uint64_t ByteSize)
{
    uint64_t PhysicalAddress = 0x20000000;
    uint64_t VirtualAddress = 0x85000000;

    MapPages(VirtualAddress, PhysicalAddress, ByteSize, false, false, false, EPhysicalState::Free);
}

void VirtualProtect(void* Address, uint64_t ByteSize, EMemoryProtection ProtectFlags)
{

}
