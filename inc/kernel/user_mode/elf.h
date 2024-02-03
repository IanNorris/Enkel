#pragma once

#include "kernel/process/process.h"

ElfBinary* LoadElfFromMemory(const char16_t* programName, const uint8_t* elfStart);
ElfBinary* LoadElf(const char16_t* programName);
void UnloadElf(ElfBinary* elfBinary);
