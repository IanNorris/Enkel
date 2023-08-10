#pragma once

#ifdef NULL
#undef NULL
#endif
#include "Uefi.h"
#ifdef NULL
#undef NULL
#endif
#define NULL nullptr
#include "Protocol/GraphicsOutput.h"
#include "kernel/framebuffer/framebuffer.h"

typedef KERNEL_API void (*BootPrintFunction)(const char16_t* message);

enum SpecialMemoryLocation
{
	SpecialMemoryLocation_KernelBinary,
	SpecialMemoryLocation_KernelStack,
	SpecialMemoryLocation_Framebuffer,
	
	SpecialMemoryLocation_MAX
};

struct KernelMemoryLocation
{
	uint64_t VirtualStart;
	uint64_t PhysicalStart;
	uint64_t ByteSize;
};

struct KernelMemoryLayout
{
	KernelMemoryLocation SpecialLocations[SpecialMemoryLocation_MAX];

	EFI_MEMORY_DESCRIPTOR* Map;
	uint32_t Entries;
	uint32_t DescriptorSize;
};

struct KernelBootData
{
	KernelMemoryLayout MemoryLayout;

	EFI_GRAPHICS_OUTPUT_PROTOCOL* GraphicsOutput;
	EFI_RUNTIME_SERVICES* RuntimeServices;
	FramebufferLayout Framebuffer;
};

extern "C"
{
	typedef void KERNEL_API /*__attribute__((__noreturn__))*/ (*KernelStartFunction)(KernelBootData* bootData);

	void __attribute__((sysv_abi, __noreturn__)) EnterKernel(KernelBootData* bootData, KernelStartFunction kernelStart, uint64_t stackPointer);
}
