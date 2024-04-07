#pragma once

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

enum class PrivilegeLevel
{
	Kernel,
	User,

	Keep //Keep the existing value (only valid if already mapped)
};

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

void* VirtualAlloc(uint64_t ByteSize, PrivilegeLevel privilegeLevel, PageFlags pageFlags = PageFlags_None);
bool VirtualFree(void* Address, uint64_t ByteSize);
void VirtualProtect(void* Address, uint64_t ByteSize, MemoryProtection ProtectFlags, PageFlags pageFlags = PageFlags_None, PrivilegeLevel privilegeLevel = PrivilegeLevel::Keep);

#define PhysicalFree VirtualFree
