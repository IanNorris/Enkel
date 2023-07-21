#pragma once

#include "fat_types.h"

enum FILE_SYSTEM_FAT : uint8_t
{
    FILE_SYSTEM_FAT_NONE,

    FILE_SYSTEM_FAT_12,
    FILE_SYSTEM_FAT_16,
    FILE_SYSTEM_FAT_32,
};

FILE_SYSTEM_FAT GetFileSystemType(const uint8_t* dataPtr);
struct direntry* GetFileCluster(const uint8_t* dataPtr, struct direntry* dirEntry, bpb33* bpb, uint32_t dirEntries, const char* filenameRemaining);
uint32_t GetNextCluster(FILE_SYSTEM_FAT fsMode, const uint8_t* dataPtr, const bpb33* bpb, uint32_t cluster);
void CopyFileToBuffer(FILE_SYSTEM_FAT fsMode, const uint8_t* dataPtr, const bpb33* bpb, uint32_t firstCluster, size_t fileSize, uint8_t* buffer, size_t bufferSize);
