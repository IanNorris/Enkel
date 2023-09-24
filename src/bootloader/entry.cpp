#include "helpers.h"
#include "bootstrap.h"
#include "memory/memory.h"
#include "common/string.h"
#include "bootloader_build.h"

#include "Protocol/GraphicsOutput.h"

#include "Protocol/LoadedImage.h"
#include "Guid/FileInfo.h"
#include "Guid/Acpi.h"

#include "IndustryStandard/Acpi61.h"


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

	SetResolution(bootServices, bootData);

	SetColor(EFI_LIGHTGREEN);
	UEFI_CALL(systemTable->ConOut, SetAttribute, EFI_LIGHTGREEN);
	Print(u"Starting Enkel (Revision ");
	Print(BOOTLOADER_BUILD_ID);
	Print(u")...\r\n");
	SetColor(EFI_WHITE);

	KernelStartFunction kernelStart = LoadKernel(imageHandle, bootServices, bootData);

	if (kernelStart == nullptr)
	{
		Halt(EFI_LOAD_ERROR, u"Failed to load kernel image");
	}

	EFI_GUID Acpi2Table = EFI_ACPI_20_TABLE_GUID;
	EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER* Acpi2Descriptor = nullptr;

	//Now find the ACPI table
	for(int systemTableEntry = 0; systemTableEntry < systemTable->NumberOfTableEntries; systemTableEntry++)
	{
		if(CompareGuids(systemTable->ConfigurationTable[systemTableEntry].VendorGuid, Acpi2Table))
		{
			//This is the RSDP
			Acpi2Descriptor = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER*)systemTable->ConfigurationTable[systemTableEntry].VendorTable;
			break;
		}
	}

	if(Acpi2Descriptor == nullptr)
	{
		Halt(0, u"System does not support ACPI 2.0");
	}

	EFI_ACPI_DESCRIPTION_HEADER* Xsdt = (EFI_ACPI_DESCRIPTION_HEADER*)Acpi2Descriptor->XsdtAddress;
	if(Xsdt == nullptr)
	{
		Halt(0, u"XSDT is null");
	}

	EFI_ACPI_SDT_HEADER* Rsdt = (EFI_ACPI_SDT_HEADER*)Acpi2Descriptor->RsdtAddress;
	if(Rsdt == nullptr)
	{
		Halt(0, u"RSDT is null");
	}

	bootData.Rsdt = Rsdt;
	bootData.Xsdt = Xsdt;

	Print(u"About to execute into the kernel...\r\n");

	ExitToKernel(bootServices, systemTable->RuntimeServices, imageHandle, bootData, kernelStart);

	//Should never get here...
}

void __attribute__((__noreturn__)) ExitToKernel(EFI_BOOT_SERVICES* bootServices, EFI_RUNTIME_SERVICES* runtimeServices, EFI_HANDLE imageHandle, KernelBootData& bootData, KernelStartFunction kernelStart)
{
	Allocation bootDataKernel = AllocatePages(EfiMemoryMapType_KernelBootData, sizeof(KernelBootData));

	UINT64 stackSize = 256 * 1024;

	//We need a new block of memory that will become the kernel's stack
	Allocation stackRange = AllocatePages(EfiMemoryMapType_KernelStack, stackSize);
	uint64_t stackHigh = (uint64_t)stackRange.Data + stackSize;

	//Need to ensure we end up in the code segment
	uint64_t APBootstrapPage = 0x1000;
	EFI_STATUS APStatus = bootServices->AllocatePages(AllocateAddress, (EFI_MEMORY_TYPE)EfiMemoryMapType_APBootstrap, 1, &APBootstrapPage);
	_ASSERTF(APStatus == EFI_SUCCESS, "Unable to allocate AP bootstrap");

	uint64_t TablePage = 0x2000;
	const int TablePageCount = 2;
	APStatus = bootServices->AllocatePages(AllocateAddress, (EFI_MEMORY_TYPE)EfiMemoryMapType_Tables, TablePageCount, &TablePage);
	_ASSERTF(APStatus == EFI_SUCCESS, "Unable to allocate tables page");

	bootData.MemoryLayout.SpecialLocations[SpecialMemoryLocation_APBootstrap].VirtualStart = APBootstrapPage;
	bootData.MemoryLayout.SpecialLocations[SpecialMemoryLocation_APBootstrap].PhysicalStart = APBootstrapPage;
	bootData.MemoryLayout.SpecialLocations[SpecialMemoryLocation_APBootstrap].ByteSize = EFI_PAGE_SIZE;

	bootData.MemoryLayout.SpecialLocations[SpecialMemoryLocation_Tables].VirtualStart = TablePage;
	bootData.MemoryLayout.SpecialLocations[SpecialMemoryLocation_Tables].PhysicalStart = TablePage;
	bootData.MemoryLayout.SpecialLocations[SpecialMemoryLocation_Tables].ByteSize = EFI_PAGE_SIZE * TablePageCount;
	
	bootData.MemoryLayout.SpecialLocations[SpecialMemoryLocation_KernelStack].VirtualStart = (uint64_t)stackRange.Data;
	bootData.MemoryLayout.SpecialLocations[SpecialMemoryLocation_KernelStack].PhysicalStart = (uint64_t)stackRange.Data;
	bootData.MemoryLayout.SpecialLocations[SpecialMemoryLocation_KernelStack].ByteSize = stackSize;

	//We need to get the memory map to say what memory ranges are used, reserved and free.
	//However, this is a bit of a convoluted process as anything we do that allocates
	//can modify the map, invalidating it. And when we exit we need to provide a "key" to
	//prove the memory map we think we have is actually the one the OS is going to exit with.

	UINTN memoryMapSize = 0;
	UINTN memoryMapSizeAllocated = 0;
	UINTN memoryMapKey = 0;

	UINTN descriptorSize = 0;
	UINT32 descriptorVersion = 0;

	//First get an initial size for it
	UINTN gmmResult = bootServices->GetMemoryMap(&memoryMapSize, nullptr, nullptr, &descriptorSize, nullptr);
	if (gmmResult != EFI_BUFFER_TOO_SMALL)
	{
		Halt(gmmResult, u"Failed to get memory map size");
	}

	Print(u"Memory map entries: ");
	char16_t tempBuffer[16];
	witoabuf(tempBuffer, memoryMapSize / descriptorSize, 10);
	Print(tempBuffer);
	Print(u"\r\n");

	EFI_MEMORY_DESCRIPTOR* memoryMap = nullptr;
	memoryMapSizeAllocated = memoryMapSize + (8 * descriptorSize);
	Allocation memoryMapAlloc = AllocatePages(EfiMemoryMapType_MemoryMap, memoryMapSizeAllocated);
	memoryMap = (EFI_MEMORY_DESCRIPTOR*)memoryMapAlloc.Data;

	int retries = 5;
	do
	{
		//Try to get the actual memory map
		memoryMapSize = memoryMapSizeAllocated;
		gmmResult = bootServices->GetMemoryMap(&memoryMapSize, memoryMap, &memoryMapKey, &descriptorSize, &descriptorVersion);

		if (gmmResult == EFI_BUFFER_TOO_SMALL)
		{
			Print(u"Memory map too small\r\n");

			FreePages(memoryMapAlloc);

			memoryMapSize = 0;
			UINTN gmmSizeResult = bootServices->GetMemoryMap(&memoryMapSize, nullptr, nullptr, &descriptorSize, nullptr);
			if (!(gmmSizeResult == EFI_SUCCESS || gmmSizeResult == EFI_BUFFER_TOO_SMALL))
			{
				Halt(gmmSizeResult, u"Failed to get memory map size (3)");
			}

			Print(u"Memory map entries: ");
			witoabuf(tempBuffer, memoryMapSize / descriptorSize, 10);
			Print(tempBuffer);
			Print(u"\r\n");

			//Add a bit of a buffer
			memoryMapSizeAllocated = memoryMapSize + (8 * descriptorSize);

			memoryMapAlloc = AllocatePages(EfiMemoryMapType_MemoryMap, memoryMapSizeAllocated);
			memoryMap = (EFI_MEMORY_DESCRIPTOR*)memoryMapAlloc.Data;
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

	Print(u"Starting kernel...\r\n");

	DrawDot(bootData); //1

	//We do one last memory map before we call exit boot services, as the map is changing...
	memoryMapSize = memoryMapSizeAllocated;
	gmmResult = bootServices->GetMemoryMap(&memoryMapSize, memoryMap, &memoryMapKey, &descriptorSize, &descriptorVersion);
	if (gmmResult != EFI_SUCCESS)
	{
		memoryMapSize = 0;
		UINTN gmmSizeResult = bootServices->GetMemoryMap(&memoryMapSize, nullptr, nullptr, &descriptorSize, nullptr);
		if (gmmSizeResult != EFI_SUCCESS)
		{
			Halt(gmmSizeResult, u"Failed to get memory map size (4)");
		}

		Print(u"Memory map entries (3): ");
		char16_t tempBuffer[16];
		witoabuf(tempBuffer, memoryMapSize / sizeof(EFI_MEMORY_DESCRIPTOR), 10);
		Print(tempBuffer);
		Print(u"\r\n");

		Halt(gmmResult, u"Failed to get memory map (pre-exit)");
	}

	DrawDot(bootData); //2

	bootData.RuntimeServices = runtimeServices;
	bootData.MemoryLayout.Map = memoryMap;
	bootData.MemoryLayout.MapSize = memoryMapSize;
	bootData.MemoryLayout.Entries = memoryMapSize / descriptorSize;
	bootData.MemoryLayout.DescriptorSize = descriptorSize;
	bootData.MemoryLayout.DescriptorVersion = descriptorVersion;

	//Now we need to find the page mapping for the framebuffer
	const KernelMemoryLayout& memoryLayout = bootData.MemoryLayout;
	KernelMemoryLocation& framebufferLocation = bootData.MemoryLayout.SpecialLocations[SpecialMemoryLocation_Framebuffer];
	framebufferLocation.PhysicalStart = (uint64_t)bootData.Framebuffer.Base; //NOTE: Framebuffer Base is a *PHYSICAL ADDRESS*.
	framebufferLocation.VirtualStart = (uint64_t)bootData.Framebuffer.Base; //It seems to need an implicit memory map entry
	framebufferLocation.ByteSize = bootData.Framebuffer.Pitch * bootData.Framebuffer.Height;

	DrawDot(bootData); //3

	ERROR_CHECK(bootServices->ExitBootServices(imageHandle, memoryMapKey), u"Error exiting boot services");

	DrawDot(bootData); //4

	//Copy the boot data to the new memory block the kernel can keep around.
	memcpy((void*)bootDataKernel.Data, &bootData, sizeof(KernelBootData));

	DrawDot(bootData); //5

	//Here we go...
	EnterKernel((KernelBootData*)bootDataKernel.Data, kernelStart, stackHigh);

	asm volatile("hlt");
	while (1);
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