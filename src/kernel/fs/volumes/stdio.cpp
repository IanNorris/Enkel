#include "common/types.h"
#include "common/string.h"
#include "memory/virtual.h"
#include "fs/volume.h"
#include "kernel/console/console.h"

#define STDOUT_HANDLES 3

//TODO: Need to lock this
char16_t ConsoleBuffer[1024];

BMFontColor StdErrColour = { 255, 0, 0 };

VolumeRead StandardInputVolume_Read = 
[](VolumeFileHandle handle, void* buffer, uint64_t size) -> uint64_t
{
	if (handle == 0)
	{
		return 0ULL;
	}

	if (size == 0)
	{
		return 0ULL;
	}	
	
	return 0ULL;
};

VolumeWrite StandardOutputVolume_Write = 
[](VolumeFileHandle handle, const void* buffer, uint64_t size) -> uint64_t
{
	const char* bufferString = (const char*)buffer;

	if (handle == 1)
	{
		return 0ULL;
	}

	uint64_t sizeRemaining = size;
	while(sizeRemaining)
	{
		int written = ascii_to_wide(ConsoleBuffer, bufferString, sizeRemaining > 1024 ? 1024 : sizeRemaining);
		ConsolePrint(ConsoleBuffer);

		sizeRemaining -= written;
	}
	
	return size;
};

VolumeWrite StandardErrorVolume_Write = 
[](VolumeFileHandle handle, const void* buffer, uint64_t size) -> uint64_t
{
	const char* bufferString = (const char*)buffer;

	if (handle == 1)
	{
		return 0ULL;
	}
	
	uint64_t sizeRemaining = size;
	while(sizeRemaining)
	{
		int written = ascii_to_wide(ConsoleBuffer, bufferString, sizeRemaining > 1024 ? 1024 : sizeRemaining);

		int32_t x = -1;
		int32_t y = -1;
		int32_t returnX = -1;
		ConsolePrintAtPosWithColor(ConsoleBuffer, x, y, returnX, StdErrColour, nullptr);

		sizeRemaining -= written;
	}
	
	return size;
};

VolumeWrite StdioWriteFunctions[STDOUT_HANDLES] = 
{
	nullptr,
	StandardOutputVolume_Write,
	StandardErrorVolume_Write
};

VolumeRead StdioReadFunctions[STDOUT_HANDLES] = 
{
	StandardInputVolume_Read,
	nullptr,
	nullptr
};

Volume StandardIOVolume
{
	OpenHandle: [](const char16_t* path, uint8_t mode){ return (VolumeFileHandle)0ULL; },
	CloseHandle: [](VolumeFileHandle handle){},
	Read: [](VolumeFileHandle handle, void* buffer, uint64_t size) -> uint64_t
	{
		if(handle < STDOUT_HANDLES)
		{
			return StdioReadFunctions[handle](handle, buffer, size);
		}
		else
		{
			return ~0ULL;
		}
	},
	Write: [](VolumeFileHandle handle, const void* buffer, uint64_t size) -> uint64_t
	{
		if(handle < STDOUT_HANDLES)
		{
			return StdioWriteFunctions[handle](handle, buffer, size);
		}
		else
		{
			return ~0ULL;
		}
	},
	Seek: nullptr,
	Tell: nullptr,
	GetSize: nullptr
};

void InitializeStdioVolumes()
{
	MountVolume(&StandardIOVolume, u"stdio");
}
