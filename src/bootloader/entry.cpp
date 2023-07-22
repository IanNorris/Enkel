#include "helpers.h"
#include "memory/memory.h"
#include "common/string.h"
#include "bootloader_build.h"

#include "Protocol/LoadedImage.h"
#include "Protocol/GraphicsOutput.h"
#include "Guid/FileInfo.h"

#include "kernel/bootload.h"

//Header guard to stop it pulling extra stuff in
#define __APPLE__
#include <elf.h>
#undef __APPLE__

KernelStartFunction LoadKernel(EFI_HANDLE imageHandle, EFI_BOOT_SERVICES* bootServices);
void SetResolution(EFI_BOOT_SERVICES* bootServices, int mode, KernelBootData& bootData);

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
	UEFI_CALL(systemTable->ConOut, Reset, 0);
	InitHelpers(systemTable);

	SetColor(EFI_LIGHTGREEN);
	UEFI_CALL(systemTable->ConOut, SetAttribute, EFI_LIGHTGREEN);
	Print(u"Starting Enkel (Revision ");
	Print(BOOTLOADER_BUILD_ID);
	Print(u")...\r\n");
	SetColor(EFI_WHITE);

	//Set a timeout of 15s, if we haven't transferred to the kernel by then we probably won't be...
	EFI_BOOT_SERVICES* bootServices = systemTable->BootServices;
	bootServices->SetWatchdogTimer(15, 0, 0, 0);

	KernelStartFunction kernelStart = LoadKernel(imageHandle, bootServices);

	if (kernelStart == nullptr)
	{
		Halt(EFI_LOAD_ERROR, u"Failed to load kernel image");
	}

	KernelBootData bootData;
	bootData.Sum = 26;
	bootData.Print = Print;

	//SetResolution(bootServices, 21, bootData);

	Print(u"\r\nAbout to execute into the kernel....\r\n");

	kernelStart(&bootData);

	char16_t tempBuffer[16];
	witoabuf(tempBuffer, bootData.Sum, 10);
	Print(u"Result: ");
	Print(tempBuffer);
	Print(u".\r\n");

	_ASSERT(bootData.Sum == 7);

	while (1)
	{
		Print(u".");
		bootServices->Stall(ONE_SECOND);
	}

	return 0;
}

// Function to validate and retrieve the entry point
KernelStartFunction PrepareKernel(EFI_BOOT_SERVICES* bootServices, const uint8_t* elfStart)
{
	const Elf64_Ehdr* elfHeader = (const Elf64_Ehdr*)elfStart;
	const Elf64_Phdr* programHeader = (const Elf64_Phdr*)((const char*)elfStart + elfHeader->e_phoff);

	//Verify the ELF is in the correct format

	if (elfHeader->e_ident[EI_MAG0] != ELFMAG0 ||
		elfHeader->e_ident[EI_MAG1] != ELFMAG1 ||
		elfHeader->e_ident[EI_MAG2] != ELFMAG2 ||
		elfHeader->e_ident[EI_MAG3] != ELFMAG3)
	{
		Halt(EFI_UNSUPPORTED,  u"Not an ELF file");
	}

	if (elfHeader->e_ident[EI_CLASS] != ELFCLASS64)
	{
		Halt(EFI_UNSUPPORTED, u"Not an ELF64 file");
	}

	if (elfHeader->e_machine != EM_X86_64)
	{
		Halt(EFI_UNSUPPORTED, u"Not an AMD64 file");
	}

	//We need to allocate memory for the kernel to be loaded into

	Elf64_Addr lowestAddress = ~0;
	Elf64_Addr highestAddress = 0;

	for (int sectionHeader = 0; sectionHeader < elfHeader->e_phnum; sectionHeader++)
	{
		const Elf64_Phdr& section = programHeader[sectionHeader];

		if (section.p_type != PT_LOAD)
		{
			continue;
		}

		if (section.p_vaddr < lowestAddress)
		{
			lowestAddress = section.p_vaddr;
		}

		if (section.p_vaddr + section.p_memsz > highestAddress)
		{
			highestAddress = section.p_vaddr + section.p_memsz;
		}
	}

	uint64_t kernelSize = highestAddress - lowestAddress;
	size_t pageCount = (kernelSize / EFI_PAGE_SIZE) + 1; //+1 as integer dividing will round down

	EFI_PHYSICAL_ADDRESS kernelStart = lowestAddress;

	char16_t tempBuffer[16];
	witoabuf(tempBuffer, kernelStart, 16);
	Print(u"Kernel to be loaded at 0x");
	Print(tempBuffer);
	Print(u".\r\n");

	witoabuf(tempBuffer, elfHeader->e_entry, 16);
	Print(u"Entry at 0x");
	Print(tempBuffer);
	Print(u".\r\n");

	witoabuf(tempBuffer, kernelSize, 16);
	Print(u"Kernel size is 0x");
	Print(tempBuffer);
	Print(u".\r\n");

	ERROR_CHECK(bootServices->AllocatePages(AllocateAddress, EfiLoaderData, pageCount, &kernelStart), u"Unable to allocate pages for the kernel");

	if (kernelStart != lowestAddress)
	{
		Halt(EFI_OUT_OF_RESOURCES, u"Kernel load address is unavailable");
	}

	for (int sectionHeader = 0; sectionHeader < elfHeader->e_phnum; sectionHeader++)
	{
		const Elf64_Phdr& section = programHeader[sectionHeader];

		if (section.p_type != PT_LOAD)
		{
			continue;
		}

		memcpy((void*)section.p_vaddr, elfStart + section.p_offset, section.p_filesz);
		memset((uint8_t*)section.p_vaddr + section.p_filesz, 0, section.p_memsz - section.p_filesz);
	}

	KernelStartFunction kernelEntry = (KernelStartFunction)elfHeader->e_entry;

	return kernelEntry;
}


KernelStartFunction LoadKernel(EFI_HANDLE imageHandle, EFI_BOOT_SERVICES* bootServices)
{
	EFI_GUID loadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	EFI_GUID simpleFileSystemProtocolGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
	EFI_GUID getInfoGuid = EFI_FILE_INFO_ID;

	EFI_LOADED_IMAGE_PROTOCOL* loadedImageService;
	ERROR_CHECK(bootServices->HandleProtocol(imageHandle, &loadedImageProtocolGuid, (void**)&loadedImageService), u"LI service");

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* simpleFileSystem;
	ERROR_CHECK(bootServices->OpenProtocol(loadedImageService->DeviceHandle, &simpleFileSystemProtocolGuid, (void**)&simpleFileSystem, imageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL), u"SFS protocol");

	EFI_FILE_PROTOCOL* volume;
	UEFI_CALL(simpleFileSystem, OpenVolume, &volume);

	EFI_FILE_PROTOCOL* kernelFile;
	UEFI_CALL(volume, Open, &kernelFile, (unsigned short*)u"KERNEL\\ENKEL.ELF", EFI_FILE_MODE_READ, 0);

	char Buffer[128];
	EFI_FILE_INFO* kernelFileInfo = (EFI_FILE_INFO*)Buffer;
	UINTN kernelInfoSize = sizeof(Buffer);

	UEFI_CALL(kernelFile, GetInfo, &getInfoGuid, &kernelInfoSize, Buffer);

	char16_t sizeBuffer[16];
	witoabuf(sizeBuffer, kernelFileInfo->FileSize / 1024, 10);

	Print(u"Kernel is ");
	Print(sizeBuffer);
	Print(u"KB.\r\n");

	uint8_t* kernelImageBuffer;
	UINTN outputBufferSize = kernelFileInfo->FileSize;

	ERROR_CHECK(bootServices->AllocatePool(EfiLoaderData, outputBufferSize, (void**)&kernelImageBuffer), u"Allocating kernel image buffer");

	UEFI_CALL(kernelFile, Read, &outputBufferSize, kernelImageBuffer);

	UEFI_CALL(kernelFile, Close);

	return PrepareKernel(bootServices, kernelImageBuffer);
}

void SetResolution(EFI_BOOT_SERVICES* bootServices, int mode, KernelBootData& bootData)
{
	EFI_GRAPHICS_OUTPUT_PROTOCOL* GraphicsOutput;

	// Get the Graphics Output Protocol instance
	EFI_GUID graphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	ERROR_CHECK(bootServices->LocateProtocol(&graphicsOutputProtocolGuid, NULL, (VOID**)&GraphicsOutput), u"Locating graphics output protocol");
	
	// Get the mode information
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info = GraphicsOutput->Mode[mode].Info;
	UEFI_CALL(GraphicsOutput, SetMode, mode);

	// Get the frame buffer base
	EFI_PHYSICAL_ADDRESS FrameBufferBase = GraphicsOutput->Mode[mode].FrameBufferBase;
	bootData.Framebuffer = (uint32_t*)FrameBufferBase;
	bootData.FramebufferWidth = GraphicsOutput->Mode[mode].Info->HorizontalResolution;
	bootData.FramebufferHeight = GraphicsOutput->Mode[mode].Info->VerticalResolution;
	bootData.FramebufferPitch = bootData.FramebufferWidth * 4;
	
	for (int i = 0; i < 1000; i++)
	{
		*(uint32_t*)FrameBufferBase = i;
		FrameBufferBase++;
	}
}
