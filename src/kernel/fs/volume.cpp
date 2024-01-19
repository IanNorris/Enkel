#include "common/types.h"
#include "common/string.h"
#include "memory/virtual.h"
#include "memory/memory.h"
#include "fs/volume.h"

#include "xxhash.h"

struct VolumePage
{
	VolumeIndex Volumes[VOLUMES_PER_PAGE];
	VolumePage* Next;
	uint64_t Unused;
} PACKED_ALIGNMENT;

VolumePage* VolumeIndices[MAX_SEGMENTS];

static_assert(sizeof(VolumeIndex) == 16, "VolumeIndex should be 16b");
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

VolumeHandle MountVolume(const Volume* volume, const char16_t* rootPath)
{
	MountPointHash rootHash = VolumeHashPath(rootPath);
	VolumePage* page = VolumeIndices[rootHash.Segments];

	for(int index = 0; index < VOLUMES_PER_PAGE; index++)
	{
		if(page->Volumes[index].VolumeImplementation == nullptr)
		{
			page->Volumes[index].RootHash = rootHash.Hash;
			page->Volumes[index].VolumeImplementation = volume;

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

void UnmountVolume(VolumeHandle handle)
{
	FileHandleMask volumeMask;
	volumeMask.FileHandle = handle;

	VolumePage* page = VolumeIndices[volumeMask.S.Segments];

	page->Volumes[volumeMask.S.VolumeIndex].VolumeImplementation = nullptr;
	page->Volumes[volumeMask.S.VolumeIndex].RootHash = 0;
}
