#pragma once

struct KernelBootData;

void BuildAndLoadPML4(const KernelBootData* bootData);
uint64_t GetPhysicalAddress(uint64_t virtualAddress);
