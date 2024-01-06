#pragma once

#include "common/types.h"
#include "kernel/init/acpi.h"

struct StackFrame
{
	StackFrame* RBP;
	uint64_t RIP;
};

extern "C" KERNEL_API StackFrame* GetCurrentStackFrame();
void __attribute__((used, noinline)) PrintStackTrace(int maxFrames, uint64_t stackRIP = 0, uint64_t stackRBP = 0);

void Halt(void);
void KERNEL_NORETURN AssertionFailed(const char* expression, const char* message, const char* filename, size_t lineNumber, uint64_t v1, uint64_t v2, uint64_t v3);
ACPI_STATUS Shutdown(void);
