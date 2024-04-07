#include "common/types.h"
#include "common/string.h"
#include "memory/virtual.h"
#include "memory/memory.h"
#include "fs/volume.h"

#include "xxhash.h"
#include <errno.h>

struct VolumePage
{
	VolumeIndex Volumes[VOLUMES_PER_PAGE];
	VolumePage* Next;
	uint64_t Unused;
} PACKED_ALIGNMENT;

VolumePage* VolumeIndices[MAX_SEGMENTS];

static_assert(sizeof(VolumeIndex) == 24, "VolumeIndex should be 16b");
static_assert(sizeof(VolumePage) == PAGE_SIZE, "VolumePage is not a page size");

void InitializeVolumeSystem()
{
	for(int index = 0; index < MAX_SEGMENTS; index++)
	{
		VolumeIndices[index] = (VolumePage*)VirtualAlloc(sizeof(VolumePage), PrivilegeLevel::Kernel);
		memset(VolumeIndices[index], 0, sizeof(VolumePage));
	}
}

MountPointHash VolumeHashPath(const char16_t* path)
{
	uint64_t stringLength = _strlen(path);

	MountPointHash out;
	out.Hash = XXH64(path, stringLength * sizeof(char16_t), 0);
	out.Segments = 0;

	for(uint64_t index = 0; index < stringLength; index++)
	{
		if(path[index] == '/')
		{
			out.Segments++;
		}
	}

	//If our path starts or ends with a slash, we don't count it as a segment
	if(stringLength > 1)
	{
		if(path[0] == '/')
		{
			out.Segments--;
		}

		if(path[stringLength - 1] == '/')
		{
			out.Segments--;
		}
	}

	if(out.Segments > MAX_SEGMENTS)
	{
		out.Segments = MAX_SEGMENTS-1;
	}

	return out;
}

VolumeHandle MountVolume(const Volume* volume, const char16_t* rootPath, void* volumeContext)
{
	MountPointHash rootHash = VolumeHashPath(rootPath);
	VolumePage* page = VolumeIndices[rootHash.Segments];

	for(int index = 0; index < VOLUMES_PER_PAGE; index++)
	{
		if(page->Volumes[index].VolumeImplementation == nullptr)
		{
			page->Volumes[index].RootHash = rootHash.Hash;
			page->Volumes[index].VolumeImplementation = volume;
			page->Volumes[index].Context = volumeContext;

			FileHandleMask handle;
			handle.S.FileHandle = 0;
			handle.S.VolumePageIndex = 0; //TODO
			handle.S.VolumeIndex = index;
			handle.S.Segments = rootHash.Segments;

			return handle.FileHandle;
		}
	}

	//TODO make new page
	_ASSERTF(false, "No free volume slots");
	return 0;
}

VolumeIndex* GetVolumeIndex(VolumeHandle handle)
{
	FileHandleMask volumeMask;
	volumeMask.FileHandle = handle;

	VolumePage* page = VolumeIndices[volumeMask.S.Segments];

	//TODO: Any kind of error checking

	return &page->Volumes[volumeMask.S.VolumeIndex];
}

void UnmountVolume(VolumeHandle handle)
{
	VolumeIndex* volumeIndex = GetVolumeIndex(handle);

	//TODO: Any kind of error checking

	volumeIndex->VolumeImplementation = nullptr;
	volumeIndex->RootHash = 0;
}

VolumeFileHandle VolumeOpenHandle(VolumeFileHandle volumeHandle, const char16_t* path, uint8_t mode)
{
	VolumeIndex* volumeIndex = volumeHandle == 0 ? nullptr : GetVolumeIndex(volumeHandle);

	if(volumeIndex == nullptr)
	{
		volumeHandle = BreakPath(path);
		volumeIndex = GetVolumeIndex(volumeHandle);
	}

	if (!(volumeIndex && volumeIndex->VolumeImplementation))
	{
		return -EINVAL;
	}

	VolumeFileHandle handle = volumeIndex->VolumeImplementation->OpenHandle(volumeHandle, volumeIndex->Context, path, mode);

	return handle;
}

void VolumeCloseHandle(VolumeFileHandle handle)
{
	VolumeIndex* volumeIndex = GetVolumeIndex(handle);

	if (!(volumeIndex && volumeIndex->VolumeImplementation))
	{
		return;
	}

	return volumeIndex->VolumeImplementation->CloseHandle(handle, volumeIndex->Context);
}

uint64_t VolumeRead(VolumeFileHandle handle, uint64_t offset, void* buffer, uint64_t size)
{
	VolumeIndex* volumeIndex = GetVolumeIndex(handle);

	if (!(volumeIndex && volumeIndex->VolumeImplementation))
	{
		return -EINVAL;
	}

	return volumeIndex->VolumeImplementation->Read(handle, volumeIndex->Context, offset, buffer, size);
}

uint64_t VolumeWrite(VolumeFileHandle handle, uint64_t offset, const void* buffer, uint64_t size)
{
	VolumeIndex* volumeIndex = GetVolumeIndex(handle);

	if (!(volumeIndex && volumeIndex->VolumeImplementation))
	{
		return -EINVAL;
	}

	return volumeIndex->VolumeImplementation->Write(handle, volumeIndex->Context, offset, buffer, size);
}

uint64_t VolumeGetSize(VolumeFileHandle handle)
{
	VolumeIndex* volumeIndex = GetVolumeIndex(handle);

	if (!(volumeIndex && volumeIndex->VolumeImplementation))
	{
		return -EINVAL;
	}

	return volumeIndex->VolumeImplementation->GetSize(handle, volumeIndex->Context);
}

uint64_t VolumeSeek(VolumeFileHandle handle, int64_t offset, SeekMode origin)
{
	VolumeIndex* volumeIndex = GetVolumeIndex(handle);

	if (!(volumeIndex && volumeIndex->VolumeImplementation))
	{
		return -EINVAL;
	}

	return volumeIndex->VolumeImplementation->Seek(handle, volumeIndex->Context, offset, origin);
}

//Pass in a path like /dev1/thing/abc and get
//back the supporting volume, and any path remaining
VolumeHandle BreakPath(const char16_t*& pathInOut)
{
	uint64_t stringLength = _strlen(pathInOut);

	MountPointHash out;

	FileHandleMask previousLongest;
	previousLongest.FileHandle = (VolumeFileHandle)0;
	const char16_t*& previousPathInOut = pathInOut;

	int segments = 0;

	for(uint64_t index = 0; index < stringLength; index++)
	{
		if(pathInOut[index] == '/')
		{
			segments++;

			out.Hash = XXH64(pathInOut, (index+1) * sizeof(char16_t), 0);

			VolumePage* page = VolumeIndices[segments];

			for(int pageIndex = 0; pageIndex < VOLUMES_PER_PAGE; pageIndex++)
			{
				if(page->Volumes[pageIndex].RootHash == out.Hash)
				{
					if(segments > MAX_SEGMENTS)
					{
						segments = MAX_SEGMENTS-1;
					}

					previousLongest.S.Segments = segments;
					previousLongest.S.VolumePageIndex = 0; //TODO
					previousLongest.S.VolumeIndex = pageIndex;
					previousPathInOut = &pathInOut[index];
				}
			}
		}
	}

	pathInOut = previousPathInOut;
	return (VolumeHandle)previousLongest.FileHandle;
}
