#pragma once

#include "common/types.h"

void Halt(void);
void DebugBreak();
void KERNEL_NORETURN HaltPermanently(void);
void KERNEL_NORETURN AssertionFailed(const char* expression, const char* message, const char* filename, size_t lineNumber);
