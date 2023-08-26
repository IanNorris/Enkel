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

void __attribute__((__noreturn__)) ExitToKernel(EFI_BOOT_SERVICES* bootServices, EFI_RUNTIME_SERVICES* runtimeServices, EFI_HANDLE imageHandle, KernelBootData& bootData, KernelStartFunction kernelStart);
bool SetResolution(EFI_BOOT_SERVICES* bootServices, KernelBootData& bootData, int desiredX, int desiredY);
void SetResolution(EFI_BOOT_SERVICES* bootServices, KernelBootData& bootData);
void DrawDot(KernelBootData& bootData);

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
	//bootServices->SetWatchdogTimer(15, 0, 0, 0);

	KernelStartFunction kernelStart = LoadKernel(imageHandle, bootServices, bootData);

	if (kernelStart == nullptr)
	{
		Halt(EFI_LOAD_ERROR, u"Failed to load kernel image");
	}

	Print(u"About to execute into the kernel...\r\n");

	ExitToKernel(bootServices, systemTable->RuntimeServices, imageHandle, bootData, kernelStart);

	//Should never get here...
}

void __attribute__((__noreturn__)) ExitToKernel(EFI_BOOT_SERVICES* bootServices, EFI_RUNTIME_SERVICES* runtimeServices, EFI_HANDLE imageHandle, KernelBootData& bootData, KernelStartFunction kernelStart)
{
	UINT64 stackSize = 256 * 1024;

	//We need a new block of memory that will become the kernel's stack
	EFI_PHYSICAL_ADDRESS stackLow = 4 * 1024 * 1024; //Must be 4K aligned if we want to enable NX later.
	UINT64 pageCount = stackSize / EFI_PAGE_SIZE;
	EFI_PHYSICAL_ADDRESS stackHigh = stackLow + stackSize;
	ERROR_CHECK(bootServices->AllocatePages(AllocateAddress, EfiLoaderData, pageCount, &stackLow), u"Failed to allocate memory for the stack");

	bootData.MemoryLayout.SpecialLocations[SpecialMemoryLocation_KernelStack].VirtualStart = stackLow;
	bootData.MemoryLayout.SpecialLocations[SpecialMemoryLocation_KernelStack].PhysicalStart = stackLow;
	bootData.MemoryLayout.SpecialLocations[SpecialMemoryLocation_KernelStack].ByteSize = stackSize;

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
	UINTN newMemoryMapSize = memoryMapSize + 5 * sizeof(EFI_MEMORY_DESCRIPTOR);
	memoryMapSize = newMemoryMapSize;

	EFI_MEMORY_DESCRIPTOR* memoryMap = nullptr;

	ERROR_CHECK(bootServices->AllocatePool(EfiLoaderData, memoryMapSize, (void**)&memoryMap), u"Failed to allocate memory for memory map");

	int retries = 5;
	do
	{
		gmmResult = bootServices->GetMemoryMap(&memoryMapSize, memoryMap, &memoryMapKey, &descriptorSize, &descriptorVersion);

		Print(u"Memory map entries (2): ");
		witoabuf(tempBuffer, memoryMapSize / sizeof(EFI_MEMORY_DESCRIPTOR), 10);
		Print(tempBuffer);
		Print(u"\r\n");

		if (gmmResult == EFI_BUFFER_TOO_SMALL)
		{
			memoryMapSize += 5 * sizeof(EFI_MEMORY_DESCRIPTOR);
		}
	} while (gmmResult == EFI_BUFFER_TOO_SMALL && retries-- > 0);

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

	DrawDot(bootData); //1

	//We do one last memory map before we call exit boot services, as the map is changing...
	memoryMapSize = newMemoryMapSize;
	gmmResult = bootServices->GetMemoryMap(&memoryMapSize, memoryMap, &memoryMapKey, &descriptorSize, &descriptorVersion);
	if (gmmResult != EFI_SUCCESS)
	{
		Halt(gmmResult, u"Failed to get memory map (pre-exit)");
	}

	DrawDot(bootData); //2

	bootData.MemoryLayout.Map = memoryMap;
	bootData.MemoryLayout.Entries = memoryMapEntries;
	bootData.MemoryLayout.DescriptorSize = descriptorSize;

	//Now we need to find the page mapping for the framebuffer
	const KernelMemoryLayout& memoryLayout = bootData.MemoryLayout;
	KernelMemoryLocation& framebufferLocation = bootData.MemoryLayout.SpecialLocations[SpecialMemoryLocation_Framebuffer];
	framebufferLocation.PhysicalStart = (uint64_t)bootData.Framebuffer.Base; //NOTE: Framebuffer Base is a *PHYSICAL ADDRESS*.
	framebufferLocation.VirtualStart = (uint64_t)bootData.Framebuffer.Base; //It seems to need an implicit memory map entry
	framebufferLocation.ByteSize = bootData.Framebuffer.Pitch * bootData.Framebuffer.Height;

	DrawDot(bootData); //3

	ERROR_CHECK(bootServices->ExitBootServices(imageHandle, memoryMapKey), u"Error exiting boot services");

	DrawDot(bootData); //4

	bootData.RuntimeServices = runtimeServices;
	//ERROR_CHECK(runtimeServices->SetVirtualAddressMap(memoryMapSize, descriptorSize, descriptorVersion, memoryMap), u"Failed to transition to virtual address mode");

	DrawDot(bootData); //5

	EnterKernel(&bootData, kernelStart, stackHigh);

	DrawDot(bootData); //6

	while (1)
	{
		bootServices->Stall(ONE_SECOND);
	}
}

void PrintStat(const char16_t* message, int value)
{
	char16_t tempBuffer[16];

	Print(message);
	witoabuf(tempBuffer, value, 10);
	Print(tempBuffer);
	Print(u"\r\n");
}

int resolutions[] = 
{
	1920, 1200,
	1920, 1080,
	1920, -1,
	1280, 800,
	1280, 720,
	128, -1,
	800, 600,
	640, 480
};

void SetResolution(EFI_BOOT_SERVICES* bootServices, KernelBootData& bootData)
{
	int resolutionPairs = (sizeof(resolutions) / sizeof(int));
	for(int res = 0; res < resolutionPairs; res += 2)
	{
		if(SetResolution(bootServices, bootData, resolutions[res], resolutions[res+1]))
		{
			return;
		}
	}

	Halt(0, u"No suitable screen resolution available");
}

bool SetResolution(EFI_BOOT_SERVICES* bootServices, KernelBootData& bootData, int desiredX, int desiredY)
{
	char16_t tempBuffer[16];

	EFI_GRAPHICS_OUTPUT_PROTOCOL* GraphicsOutput;

	// Get the Graphics Output Protocol instance
	EFI_GUID graphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	ERROR_CHECK(bootServices->LocateProtocol(&graphicsOutputProtocolGuid, NULL, (VOID**)&GraphicsOutput), u"Locating graphics output protocol");
	
	UINTN InfoSize = 0;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info = nullptr;

	int mode = 0;
	int selectedMode = -1;
	int currentResolutionY = 0;
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* Mode = GraphicsOutput->Mode;
	while (mode < Mode->MaxMode)
	{
		UEFI_CALL(GraphicsOutput, QueryMode, mode, &InfoSize, &Info);

		if (Info->HorizontalResolution == desiredX && (Info->VerticalResolution == desiredY || desiredY == -1) && Info->VerticalResolution >= currentResolutionY)
		{
			selectedMode = mode;
			currentResolutionY = Info->HorizontalResolution;
		}
		mode++;
	}

	if (selectedMode == -1)
	{
		return false;
	}

	// Get the mode information
	Info = GraphicsOutput->Mode[mode].Info;
	UEFI_CALL(GraphicsOutput, SetMode, selectedMode);

	// Get the frame buffer base
	EFI_PHYSICAL_ADDRESS FrameBufferBase = GraphicsOutput->Mode[0].FrameBufferBase;
	bootData.GraphicsOutput = GraphicsOutput;
	bootData.Framebuffer.Base = (uint32_t*)FrameBufferBase;
	bootData.Framebuffer.Width = GraphicsOutput->Mode[0].Info->HorizontalResolution;
	bootData.Framebuffer.Height = GraphicsOutput->Mode[0].Info->VerticalResolution;
	bootData.Framebuffer.Pitch = GraphicsOutput->Mode[0].Info->PixelsPerScanLine * 4;

	return true;
}

int dotIndex = 0;

void DrawDot(KernelBootData& bootData)
{
	uint32_t* base = bootData.Framebuffer.Base;
	uint32_t pitchPixels = bootData.Framebuffer.Pitch / 4;

	int dotSize = 5;

	int leftOffset = 100 + (dotIndex * 2) * dotSize;
	int startY = 100;
	for(int x = 0; x < dotSize; x++)
	{
		for(int y = 0; y < dotSize; y++)
		{
			uint32_t outputPos = ((y + startY) * pitchPixels) + x + leftOffset;
			base[outputPos] = 0x0000FF00;
		}
	}

	dotIndex++;
}