#include "common/types.h"
#include "common/string.h"
#include "memory/virtual.h"
#include "memory/memory.h"
#include "fs/volume.h"
#include "kernel/console/console.h"
#include "errno.h"

extern const char16_t* KernelBuildId;

struct SpecialPathEntry
{
	const char16_t* Path;
	const char* Data;
};

static const SpecialPathEntry SpecialPaths[] =
{
	{ u"/proc/sys/kernel/osrelease", "dev" },

	{ nullptr, nullptr }
};

Volume SpecialProcVolume
{
	OpenHandle: [](VolumeFileHandle volumeHandle, void* context, const char16_t* path, uint8_t mode)
	{
		FileHandleMask Mask;
		Mask.FileHandle = volumeHandle;

		int index = 0;
		while (SpecialPaths[index].Path)
		{
			if (_stricmp(SpecialPaths[index].Path, path) == 0)
			{
				Mask.S.FileHandle = index;
				return Mask.FileHandle;
			}

			index++;
		}

		return (uint64_t)-ENOENT;
	},
	CloseHandle : [](VolumeFileHandle handle, void* context) {},
	Read : [](VolumeFileHandle handle, void* context, uint64_t offset, void* buffer, uint64_t size) -> uint64_t
	{
		FileHandleMask Mask;
		Mask.FileHandle = handle;
		
		//TODO Range check
		const char* data = SpecialPaths[Mask.S.FileHandle].Data;

		//TODO Range check
		data += offset;

		uint64_t bytesLeft = strlen(data);
		uint64_t toCopy = min(bytesLeft, size);

		memcpy(buffer, data, toCopy);
		
		return toCopy;
	},
	Write: nullptr,
	GetSize: nullptr,
	Seek : [](VolumeFileHandle handle, void* context, int64_t offset, SeekMode origin) -> uint64_t { return -EINVAL; }
};

void InitializeSpecialProcVolumes()
{
	MountVolume(&SpecialProcVolume, u"/proc/", (void*)0);
}
