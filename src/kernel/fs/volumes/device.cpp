#include "common/types.h"
#include "common/string.h"
#include "memory/virtual.h"
#include "memory/memory.h"
#include "fs/volume.h"
#include "kernel/console/console.h"
#include "errno.h"

extern Volume FramebufferVolume;

struct SpecialPathEntry
{
	const char16_t* Path;
	const Volume* VolumeObject;
};

static const SpecialPathEntry SpecialPaths[] =
{
	{ u"/fb0", &FramebufferVolume },

	{ nullptr, nullptr }
};

Volume DeviceVolume
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
				
				//TODO Range check
				const Volume* volume = SpecialPaths[Mask.S.FileHandle].VolumeObject;

				return volume->OpenHandle(Mask.FileHandle, context, path, mode);
			}

			index++;
		}

		return (uint64_t)-ENOENT;
	},
	CloseHandle : [](VolumeFileHandle handle, void* context)
	{
		FileHandleMask Mask;
		Mask.FileHandle = handle;

		//TODO Range check
		const Volume* volume = SpecialPaths[Mask.S.FileHandle].VolumeObject;

		return volume->CloseHandle(handle, context);
	},
	Read : [](VolumeFileHandle handle, void* context, uint64_t offset, void* buffer, uint64_t size) -> uint64_t
	{
		FileHandleMask Mask;
		Mask.FileHandle = handle;

		//TODO Range check
		const Volume* volume = SpecialPaths[Mask.S.FileHandle].VolumeObject;

		return volume->Read(handle, context, offset, buffer, size);
	},
	Write: [](VolumeFileHandle handle, void* context, uint64_t offset, const void* buffer, uint64_t size) -> uint64_t
	{
		FileHandleMask Mask;
		Mask.FileHandle = handle;

		//TODO Range check
		const Volume* volume = SpecialPaths[Mask.S.FileHandle].VolumeObject;

		return volume->Write(handle, context, offset, buffer, size);
	},
	GetSize : [](VolumeFileHandle handle, void* context)->uint64_t
	{
		FileHandleMask Mask;
		Mask.FileHandle = handle;

		//TODO Range check
		const Volume* volume = SpecialPaths[Mask.S.FileHandle].VolumeObject;

		return volume->GetSize(handle, context);
	},
	Seek : [](VolumeFileHandle handle, void* context, int64_t offset, SeekMode origin) -> uint64_t
	{
		FileHandleMask Mask;
		Mask.FileHandle = handle;

		//TODO Range check
		const Volume* volume = SpecialPaths[Mask.S.FileHandle].VolumeObject;

		return volume->Seek(handle, context, offset, origin);
	},
	Command : [](VolumeFileHandle handle, void* context, uint64_t command, uint64_t data) -> uint64_t
	{
		FileHandleMask Mask;
		Mask.FileHandle = handle;

		//TODO Range check
		const Volume* volume = SpecialPaths[Mask.S.FileHandle].VolumeObject;

		if (volume->Command)
		{
			return volume->Command(handle, context, command, data);
		}
		else
		{
			return -EINVAL;
		}
	}
};

void InitializeDeviceVolumes()
{
	MountVolume(&DeviceVolume, u"/dev/", nullptr);
}
