#pragma once

#include <stdint.h>
#include <stddef.h>

enum class MemoryProtection
{
    ReadOnly,
    ReadWrite,
    Execute,
};

void* VirtualAlloc(uint64_t ByteSize);
void VirtualFree(void* Address, uint64_t ByteSize);
void VirtualProtect(void* Address, uint64_t ByteSize, MemoryProtection ProtectFlags);

#define PhysicalFree VirtualFree
