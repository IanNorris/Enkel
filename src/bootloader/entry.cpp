#include "helpers.h"
#include "bootstrap.h"
#include "memory/memory.h"
#include "common/string.h"
#include "bootloader_build.h"

#include "Protocol/GraphicsOutput.h"

#include "Protocol/LoadedImage.h"
#include "Guid/FileInfo.h"


//Header guard to stop it pulling extra stuff in
#define __APPLE__
#include <elf.h>
#undef __APPLE__

void __attribute__((__noreturn__)) ExitToKernel(EFI_BOOT_SERVICES* bootServices, EFI_HANDLE imageHandle, KernelBootData& bootData, KernelStartFunction kernelStart);
void SetResolution(EFI_BOOT_SERVICES* bootServices, KernelBootData& bootData);

EFI_STATUS __attribute__((__noreturn__)) efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
	UEFI_CALL(systemTable->ConOut, Reset, 0);
	InitHelpers(systemTable);

	KernelBootData bootData;

	EFI_BOOT_SERVICES* bootServices = systemTable->BootServices;

	SetColor(EFI_LIGHTGREEN);
	UEFI_CALL(systemTable->ConOut, SetAttribute, EFI_LIGHTGREEN);
	Print(u"Starting Enkel (Revision ");
	Print(BOOTLOADER_BUILD_ID);
	Print(u")...\r\n");
	SetColor(EFI_WHITE);

	//Set a timeout of 15s, if we haven't transferred to the kernel by then we probably won't be...
	bootServices->SetWatchdogTimer(15, 0, 0, 0);

	KernelStartFunction kernelStart = LoadKernel(imageHandle, bootServices);

	if (kernelStart == nullptr)
	{
		Halt(EFI_LOAD_ERROR, u"Failed to load kernel image");
	}

	Print(u"About to execute into the kernel...\r\n");

	ExitToKernel(bootServices, imageHandle, bootData, kernelStart);

	//Should never get here...
}

void __attribute__((__noreturn__)) ExitToKernel(EFI_BOOT_SERVICES* bootServices, EFI_HANDLE imageHandle, KernelBootData& bootData, KernelStartFunction kernelStart)
{
	UINTN memoryMapSize = 0;
	UINTN memoryMapKey = 0;

	UINTN descriptorSize = 0;
	UINT32 descriptorVersion = 0;

	UINTN gmmResult = bootServices->GetMemoryMap(&memoryMapSize, nullptr, nullptr, &descriptorSize, nullptr);
	if (gmmResult != EFI_BUFFER_TOO_SMALL)
	{
		Halt(gmmResult, u"Failed to get memory map size");
	}

	Print(u"Memory map entries: ");
	char16_t tempBuffer[16];
	witoabuf(tempBuffer, memoryMapSize / sizeof(EFI_MEMORY_DESCRIPTOR), 10);
	Print(tempBuffer);
	Print(u"\r\n");

	// We're about to allocate memory, which can fragment the memory map.
	// So create some extra space for the number of entries we might need.
	UINTN newMemoryMapSize = memoryMapSize + 2 * sizeof(EFI_MEMORY_DESCRIPTOR);
	memoryMapSize = newMemoryMapSize;

	EFI_MEMORY_DESCRIPTOR* memoryMap = nullptr;

	ERROR_CHECK(bootServices->AllocatePool(EfiLoaderData, memoryMapSize, (void**)&memoryMap), u"Failed to allocate memory for memory map");

	gmmResult = bootServices->GetMemoryMap(&memoryMapSize, memoryMap, &memoryMapKey, &descriptorSize, &descriptorVersion);

	Print(u"Memory map entries (2): ");
	witoabuf(tempBuffer, memoryMapSize / sizeof(EFI_MEMORY_DESCRIPTOR), 10);
	Print(tempBuffer);
	Print(u"\r\n");

	if (gmmResult != EFI_SUCCESS)
	{
		Halt(gmmResult, u"Failed to get memory map");
	}

	uint64_t totalMemoryPages = 0;

	uint64_t memoryMapEntries = memoryMapSize / descriptorSize;
	for (int entry = 0; entry < memoryMapEntries; entry++)
	{
		if (memoryMap[entry].Type == EfiConventionalMemory)
		{
			totalMemoryPages += memoryMap[entry].NumberOfPages;
		}
	}

	uint64_t totalMemoryBytes = totalMemoryPages * EFI_PAGE_SIZE;
	uint64_t totalMemoryUnits = totalMemoryBytes;
	const char16_t* unit = u"B";
	if (totalMemoryUnits > 1024)
	{
		unit = u"KB";
		totalMemoryUnits /= 1024;
	}
	if (totalMemoryUnits > 1024)
	{
		unit = u"MB";
		totalMemoryUnits /= 1024;
	}
	if (totalMemoryUnits > 1024)
	{
		unit = u"GB";
		totalMemoryUnits /= 1024;
	}
	if (totalMemoryUnits > 1024)
	{
		unit = u"TB";
		totalMemoryUnits /= 1024;
	}

	Print(u"Memory available: ");
	witoabuf(tempBuffer, totalMemoryUnits, 10);
	Print(tempBuffer);
	Print(unit);
	Print(u"\r\n");

	Print(u"Starting kernel...\r\n");

	SetResolution(bootServices, bootData);

	//We do one last memory map before we call exit boot services, as the map is changing...
	memoryMapSize = newMemoryMapSize;
	gmmResult = bootServices->GetMemoryMap(&memoryMapSize, memoryMap, &memoryMapKey, &descriptorSize, &descriptorVersion);
	if (gmmResult != EFI_SUCCESS)
	{
		Halt(gmmResult, u"Failed to get memory map (pre-exit)");
	}

	bootData.MemoryLayout.Map = memoryMap;
	bootData.MemoryLayout.Entries = memoryMapEntries;
	bootData.MemoryLayout.DescriptorSize = descriptorSize;

	ERROR_CHECK(bootServices->ExitBootServices(imageHandle, memoryMapKey), u"Error exiting boot services");

	kernelStart(&bootData);
}

void PrintStat(const char16_t* message, int value)
{
	char16_t tempBuffer[16];

	Print(message);
	witoabuf(tempBuffer, value, 10);
	Print(tempBuffer);
	Print(u"\r\n");
}

void SetResolution(EFI_BOOT_SERVICES* bootServices, KernelBootData& bootData)
{
	int desiredResolutionX = 1920;
	int desiredResolutionY = 1080;

	char16_t tempBuffer[16];

	EFI_GRAPHICS_OUTPUT_PROTOCOL* GraphicsOutput;

	// Get the Graphics Output Protocol instance
	EFI_GUID graphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	ERROR_CHECK(bootServices->LocateProtocol(&graphicsOutputProtocolGuid, NULL, (VOID**)&GraphicsOutput), u"Locating graphics output protocol");
	
	PrintStat(u"Display modes: ", GraphicsOutput->Mode[0].MaxMode);

	UINTN InfoSize = 0;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info = nullptr;

	int mode = 0;
	int selectedMode = -1;
	int currentResolutionX = 0;
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* Mode = GraphicsOutput->Mode;
	while (mode < Mode->MaxMode)
	{
		UEFI_CALL(GraphicsOutput, QueryMode, mode, &InfoSize, &Info);

		if (Info->HorizontalResolution <= desiredResolutionX && Info->VerticalResolution <= desiredResolutionY && Info->HorizontalResolution >= currentResolutionX)
		{
			selectedMode = mode;
			currentResolutionX = Info->HorizontalResolution;
		}
		mode++;
	}

	if (selectedMode != -1)
	{
		PrintStat(u"Picked display mode: ", mode);
		PrintStat(u"--Width: ", Info->HorizontalResolution);
		PrintStat(u"--Height: ", Info->VerticalResolution);
		PrintStat(u"--Pitch: ", Info->PixelsPerScanLine);
		PrintStat(u"--PixelFormat: ", Info->PixelFormat);
	}
	else
	{
		Halt(10, u"Failed to find a suitable graphics mode");
	}

	// Get the mode information
	Info = GraphicsOutput->Mode[mode].Info;
	UEFI_CALL(GraphicsOutput, SetMode, selectedMode);

	// Get the frame buffer base
	EFI_PHYSICAL_ADDRESS FrameBufferBase = GraphicsOutput->Mode[0].FrameBufferBase;
	bootData.GraphicsOutput = GraphicsOutput;
	bootData.Framebuffer = (uint32_t*)FrameBufferBase;
	bootData.FramebufferWidth = GraphicsOutput->Mode[0].Info->HorizontalResolution;
	bootData.FramebufferHeight = GraphicsOutput->Mode[0].Info->VerticalResolution;
	bootData.FramebufferPitch = GraphicsOutput->Mode[0].Info->PixelsPerScanLine * 4;
}
