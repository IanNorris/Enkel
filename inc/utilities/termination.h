#pragma once

#include "common/types.h"

struct StackFrame
{
	StackFrame* RBP;
	uint64_t RIP;
};

extern "C" KERNEL_API StackFrame* GetCurrentStackFrame();
void __attribute__((used, noinline)) PrintStackTrace(int maxFrames);

void Halt(void);
void KERNEL_NORETURN AssertionFailed(const char* expression, const char* message, const char* filename, size_t lineNumber, uint64_t v1, uint64_t v2, uint64_t v3);
