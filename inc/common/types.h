#pragma once

#include <stdint.h>
#include <stddef.h>

#ifndef __cplusplus
#define bool uint8_t
#define nullptr 0
#define true 1
#define false 0
#endif

#ifndef BOOL
#define BOOL uint8_t
#endif

#define KERNEL_API __attribute__((sysv_abi))

extern "C" void AssertionFailed(const char* expression, const char* message, const char* filename, size_t lineNumber);

#define _ASSERT(x) _ASSERTF(x, nullptr)
#define _ASSERTF(x, message) if( !(x) ) { AssertionFailed(#x, message, __FILE__, __LINE__); }
