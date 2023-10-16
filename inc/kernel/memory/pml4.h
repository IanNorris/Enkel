#pragma once

#include <memory/physical.h>

struct KernelBootData;

void AllocateNextFreePageTableEntries();

void MapPages(uint64_t virtualAddress, uint64_t physicalAddress, uint64_t size, bool writable, bool executable, MemoryState::RangeState newState, PageFlags pageFlags = PageFlags_None);

void BuildAndLoadPML4(KernelBootData* bootData);
void MemCheck(KernelBootData* bootData);
