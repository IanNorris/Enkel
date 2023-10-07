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
#define KERNEL_NORETURN __attribute__((noreturn))

#define DebugBreak() asm volatile("int $3")

extern "C" void AssertionFailed(const char* expression, const char* message, const char* filename, size_t lineNumber, uint64_t v1, uint64_t v2, uint64_t v3);

#define _ASSERT(x) _ASSERTF(x, nullptr)
#define _ASSERTF(x, message) if( !(x) ) { AssertionFailed(#x, message, __FILE__, __LINE__, 0, 0, 0); DebugBreak(); }
#define _ASSERTFV(x, message, v1, v2, v3) if( !(x) ) { AssertionFailed(#x, message, __FILE__, __LINE__, v1, v2, v3); DebugBreak(); }
#define NOT_IMPLEMENTED() _ASSERTF(false, "Not implemented")