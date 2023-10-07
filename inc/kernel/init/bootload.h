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
#include "IndustryStandard/Acpi61.h"
#include "Protocol/AcpiSystemDescriptionTable.h"

#include "kernel/framebuffer/framebuffer.h"

typedef KERNEL_API void (*BootPrintFunction)(const char16_t* message);

enum EfiMemoryMapType
{
	EfiMemoryMapType_MemoryMap = EfiLoaderCode, // = 0x80000000,
	EfiMemoryMapType_KernelBootData = EfiLoaderData,
	EfiMemoryMapType_Kernel = EfiLoaderCode,
	EfiMemoryMapType_KernelStack = EfiLoaderData,
	EfiMemoryMapType_APBootstrap = EfiLoaderCode,
	EfiMemoryMapType_Tables = EfiLoaderData,
	EfiMemoryMapType_Framebuffer = EfiRuntimeServicesData,
};

enum SpecialMemoryLocation
{
	SpecialMemoryLocation_KernelBinary,
	SpecialMemoryLocation_KernelStack,
	SpecialMemoryLocation_Framebuffer,
	SpecialMemoryLocation_APBootstrap,
	SpecialMemoryLocation_Tables,
	
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
	uint32_t MapSize;
	uint32_t Entries;
	uint32_t DescriptorSize;
	uint32_t DescriptorVersion;
};

struct KernelBootData
{
	KernelMemoryLayout MemoryLayout;

	EFI_GRAPHICS_OUTPUT_PROTOCOL* GraphicsOutput;
	EFI_RUNTIME_SERVICES* RuntimeServices;
	EFI_ACPI_DESCRIPTION_HEADER* Xsdt;
	EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER* Rsdt;
	FramebufferLayout Framebuffer;
};

extern "C"
{
	typedef void KERNEL_API /*__attribute__((__noreturn__))*/ (*KernelStartFunction)(KernelBootData* bootData);

	void __attribute__((sysv_abi, __noreturn__)) EnterKernel(KernelBootData* bootData, KernelStartFunction kernelStart, uint64_t stackPointer);
}
