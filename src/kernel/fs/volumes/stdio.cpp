#include "common/types.h"
#include "common/string.h"
#include "common/ring_buffer.h"
#include "memory/virtual.h"
#include "fs/volume.h"
#include "kernel/console/console.h"
#include "errno.h"

#define STDOUT_HANDLES 3

//TODO: Need to lock this
char16_t ConsoleBuffer[1024];

constexpr size_t InputBufferLength = 4 * 1024;
RingBuffer<char, InputBufferLength> InputBuffer;

void InsertInput(char input)
{
	InputBuffer.Push(input);
}

BMFontColor StdErrColour = { 255, 0, 0 };

VolumeReadType StandardInputVolume_Read = 
[](VolumeFileHandle handle, void* context, uint64_t offset, void* buffer, uint64_t size) -> uint64_t
{
	if (size == 0)
	{
		return 0ULL;
	}

	if (handle == 0)
	{	
		size_t read;
		
		do
		{
			read = InputBuffer.PopElements((char*)buffer, size);
			if (read == 0)
			{
				asm("sti");

				asm("hlt");
			}
		} while (read == 0);

		return read;
	}

	return 0ULL;
};

VolumeWriteType StandardOutputVolume_Write = 
[](VolumeFileHandle handle, void* context, uint64_t offset, const void* buffer, uint64_t size) -> uint64_t
{
	const char* bufferString = (const char*)buffer;

	uint64_t sizeRemaining = size;
	while(sizeRemaining)
	{
		int written = ascii_to_wide(ConsoleBuffer, bufferString, sizeRemaining > 1024 ? 1024 : sizeRemaining);
		ConsolePrint(ConsoleBuffer);

		sizeRemaining -= written;
	}
	
	return size;
};

VolumeWriteType StandardErrorVolume_Write = 
[](VolumeFileHandle handle, void* context, uint64_t offset, const void* buffer, uint64_t size) -> uint64_t
{
	const char* bufferString = (const char*)buffer;
	
	uint64_t sizeRemaining = size;
	while(sizeRemaining)
	{
		int written = ascii_to_wide(ConsoleBuffer, bufferString, sizeRemaining > 1024 ? 1024 : sizeRemaining);

		int32_t x = -1;
		int32_t y = -1;
		int32_t returnX = -1;
		ConsolePrintAtPosWithColor(ConsoleBuffer, x, y, returnX, StdErrColour, nullptr);
		ConsoleSetPos(x,y);

		sizeRemaining -= written;
	}
	
	return size;
};

VolumeWriteType StdioWriteFunctions[STDOUT_HANDLES] = 
{
	nullptr,
	StandardOutputVolume_Write,
	StandardErrorVolume_Write
};

VolumeReadType StdioReadFunctions[STDOUT_HANDLES] = 
{
	StandardInputVolume_Read,
	nullptr,
	nullptr
};

Volume StandardIOVolume
{
	OpenHandle: [](VolumeFileHandle volumeHandle, void* context, const char16_t* path, uint8_t mode){ return (VolumeFileHandle)0ULL; },
	CloseHandle: [](VolumeFileHandle handle, void* context){},
	Read: [](VolumeFileHandle handle, void* context, uint64_t offset, void* buffer, uint64_t size) -> uint64_t
	{
		if(handle < STDOUT_HANDLES)
		{
			return StdioReadFunctions[handle](handle, context, offset, buffer, size);
		}
		else
		{
			return ~0ULL;
		}
	},
	Write: [](VolumeFileHandle handle, void* context, uint64_t offset, const void* buffer, uint64_t size) -> uint64_t
	{
		if(handle < STDOUT_HANDLES)
		{
			return StdioWriteFunctions[handle](handle, context, offset, buffer, size);
		}
		else
		{
			return ~0ULL;
		}
	},
	GetSize: [](VolumeFileHandle handle, void* context) -> uint64_t { return -EINVAL; }
};

void InitializeStdioVolumes()
{
	MountVolume(&StandardIOVolume, u"stdin", (void*)0);
	MountVolume(&StandardIOVolume, u"stdout", (void*)1);
	MountVolume(&StandardIOVolume, u"stderr", (void*)2);
}
