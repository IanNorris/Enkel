#pragma once

#include "fs/volume.h"

struct FileInternal
{
	VolumeHandle Volume;
	VolumeFileHandle File;
};

#define VOLUME_HANDLE_INVALID (~0ULL)

FileHandle OpenFile(const char16_t* path, uint64_t mode);
void MountFatVolume(const char16_t* mountPoint, VolumeHandle volume);
