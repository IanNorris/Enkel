#include "common/types.h"
#include "common/string.h"
#include "common/ring_buffer.h"
#include "memory/virtual.h"
#include "fs/volume.h"
#include "kernel/console/console.h"
#include "errno.h"

#define STDOUT_HANDLES 4

//TODO: Need to lock this
char16_t ConsoleBuffer[2048];

constexpr size_t InputBufferLength = 4 * 1024;
RingBuffer<uint8_t, InputBufferLength> InputBuffer;
RingBuffer<uint8_t, InputBufferLength> InputScanCodeBuffer;

void InsertInput(uint8_t input, bool scancode)
{
	if (scancode)
	{
		InputScanCodeBuffer.Push(input);
	}
	else
	{
		InputBuffer.Push(input);
	}
}

void ClearInput(bool scancode)
{
	if (scancode)
	{
		InputScanCodeBuffer.Clear();
	}
	else
	{
		InputBuffer.Clear();
	}
}

size_t ReadInputNoBlocking(uint8_t* buffer, size_t size, bool scancode)
{
	if (scancode)
	{
		return InputScanCodeBuffer.PopElements(buffer, size);
	}
	else
	{
		return InputBuffer.PopElements(buffer, size);
	}
}

size_t ReadInputBlocking(uint8_t* buffer, size_t size, bool scancode)
{
	size_t read;

	do
	{
		read = ReadInputNoBlocking(buffer, size, scancode);
		if (read == 0)
		{
			asm("sti");

			asm("hlt");
		}
	} while (read == 0);

	return read;
}

BMFontColor StdErrColour = { 255, 0, 0 };

VolumeReadType StandardInputVolume_Read = 
[](VolumeFileHandle handle, void* context, uint64_t offset, void* buffer, uint64_t size) -> uint64_t
{
	if (size == 0)
	{
		return 0ULL;
	}

	if ((uint64_t)handle == 0)
	{
		return ReadInputBlocking((uint8_t*)buffer, size, false);
	}
	else
	{
		return ReadInputNoBlocking((uint8_t*)buffer, size, true);
	}
};

VolumeWriteType StandardOutputVolume_Write = 
[](VolumeFileHandle handle, void* context, uint64_t offset, const void* buffer, uint64_t size) -> uint64_t
{
	const uint8_t* bufferString = (const uint8_t*)buffer;

	uint64_t sizeRemaining = size;
	while(sizeRemaining)
	{
		int written = ascii_to_wide(ConsoleBuffer, (const char*)bufferString, sizeRemaining > 1024 ? 1024 : sizeRemaining);
		ConsolePrint(ConsoleBuffer);

		sizeRemaining -= written;
	}
	
	return size;
};

VolumeWriteType StandardErrorVolume_Write = 
[](VolumeFileHandle handle, void* context, uint64_t offset, const void* buffer, uint64_t size) -> uint64_t
{
	const uint8_t* bufferString = (const uint8_t*)buffer;
	
	uint64_t sizeRemaining = size;
	while(sizeRemaining)
	{
		int written = ascii_to_wide(ConsoleBuffer, (const char*)bufferString, sizeRemaining > 1024 ? 1024 : sizeRemaining);

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
	StandardErrorVolume_Write,
	nullptr,
};

VolumeReadType StdioReadFunctions[STDOUT_HANDLES] = 
{
	StandardInputVolume_Read,
	nullptr,
	nullptr,
	StandardInputVolume_Read,
};

Volume StandardIOVolume
{
	OpenHandle: [](VolumeFileHandle volumeHandle, void* context, const char16_t* path, uint8_t mode)
	{
		if (path[0] == 'k')
		{
			return (VolumeFileHandle)3;
		}
		else
		{
			return (VolumeFileHandle)0;
		}
	},
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
			return StdioWriteFunctions[handle](handle, (uint8_t*)context, offset, buffer, size);
		}
		else
		{
			return ~0ULL;
		}
	},
	GetSize: [](VolumeFileHandle handle, void* context) -> uint64_t { return -EINVAL; },
	Seek: [](VolumeFileHandle handle, void* context, int64_t offset, SeekMode origin) -> uint64_t { return -EINVAL; },
	Command: nullptr,
};

void InitializeStdioVolumes()
{
	MountVolume(&StandardIOVolume, u"stdin", (void*)0);
	MountVolume(&StandardIOVolume, u"stdout", (void*)1);
	MountVolume(&StandardIOVolume, u"stderr", (void*)2);
	MountVolume(&StandardIOVolume, u"keyboard", (void*)3);
}
