#pragma once

#include <stdint.h>
#include <stddef.h>

enum PageFlags
{
	PageFlags_None,
	PageFlags_Cache_WriteThrough,
	PageFlags_Cache_Disable,
};

enum class MemoryProtection
{
    ReadOnly,
    ReadWrite,
    Execute,
};

void* VirtualAlloc(uint64_t ByteSize);
bool VirtualFree(void* Address, uint64_t ByteSize);
void VirtualProtect(void* Address, uint64_t ByteSize, MemoryProtection ProtectFlags, PageFlags pageFlags = PageFlags_None);

#define PhysicalFree VirtualFree
