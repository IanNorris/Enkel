#pragma once

struct KernelBootData;

void BuildAndLoadPML4(KernelBootData* bootData);
uint64_t GetPhysicalAddress(uint64_t virtualAddress);
