#pragma once

#include "common/types.h"

extern "C" KERNEL_API void SetTLSBase(void* TlsBase);
void InitializeTLS();