#pragma once

#include "Uefi.h"
#include "Protocol/GraphicsOutput.h"

typedef KERNEL_API void (*BootPrintFunction)(const char16_t* message);

struct KernelMemoryLayout
{
	EFI_MEMORY_DESCRIPTOR* Map;
	uint32_t Entries;
	uint32_t DescriptorSize;
};

struct KernelBootData
{
	KernelMemoryLayout MemoryLayout;

	EFI_GRAPHICS_OUTPUT_PROTOCOL* GraphicsOutput;
	uint32_t* Framebuffer;
	unsigned int FramebufferWidth;
	unsigned int FramebufferHeight;
	unsigned int FramebufferPitch;
};

extern "C"
{
	typedef void KERNEL_API /*__attribute__((__noreturn__))*/ (*KernelStartFunction)(KernelBootData* bootData);
}
