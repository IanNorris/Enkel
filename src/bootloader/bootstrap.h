#pragma once

#include "kernel/init/bootload.h"

KernelStartFunction PrepareKernel(EFI_BOOT_SERVICES* bootServices, const uint8_t* elfStart);
KernelStartFunction LoadKernel(EFI_HANDLE imageHandle, EFI_BOOT_SERVICES* bootServices);
