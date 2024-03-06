#pragma once

#include "kernel/process/process.h"
#include "fs/volume.h"

ElfBinary* LoadElfFromMemory(const char16_t* programName, const uint8_t* elfStart);
ElfBinary* LoadElfFromHandle(const char16_t* programName, VolumeFileHandle handle);
ElfBinary* LoadElf(const char16_t* programName);
void UnloadElf(ElfBinary* elfBinary);
