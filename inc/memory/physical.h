#pragma once

#include "memory/virtual.h"
#include "kernel/memory/state.h"

enum PageFlags
{
	PageFlags_None,
	PageFlags_Cache_WriteThrough,
	PageFlags_Cache_Disable,
};

void* PhysicalAlloc(uint64_t PhysicalAddress, uint64_t ByteSize, PageFlags pageFlags = PageFlags_None);
void* PhysicalAllocLowestAddress(uint64_t ByteSize, PageFlags pageFlags = PageFlags_None);
uint64_t GetPhysicalAddress(uint64_t virtualAddress); //Defined in pml4.cpp
#define PhysicalFree VirtualFree
