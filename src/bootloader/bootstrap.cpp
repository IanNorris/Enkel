#include "helpers.h"
#include "bootstrap.h"
#include "memory/memory.h"
#include "common/string.h"

#include "Protocol/LoadedImage.h"
#include "Guid/FileInfo.h"

//Header guard to stop it pulling extra stuff in
#define __APPLE__
#include <elf.h>
#undef __APPLE__

// Function to validate and retrieve the entry point
KernelStartFunction PrepareKernel(EFI_BOOT_SERVICES* bootServices, const uint8_t* elfStart, KernelBootData& bootData)
{
	const Elf64_Ehdr* elfHeader = (const Elf64_Ehdr*)elfStart;
	const Elf64_Phdr* programHeader = (const Elf64_Phdr*)((const char*)elfStart + elfHeader->e_phoff);

	//Verify the ELF is in the correct format

	if (elfHeader->e_ident[EI_MAG0] != ELFMAG0 ||
		elfHeader->e_ident[EI_MAG1] != ELFMAG1 ||
		elfHeader->e_ident[EI_MAG2] != ELFMAG2 ||
		elfHeader->e_ident[EI_MAG3] != ELFMAG3)
	{
		Halt(EFI_UNSUPPORTED, u"Not an ELF file");
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

	ERROR_CHECK(bootServices->AllocatePages(AllocateAddress, EfiLoaderData, pageCount, &kernelStart), u"Unable to allocate pages for the kernel");

	bootData.MemoryLayout.SpecialLocations[SpecialMemoryLocation_KernelBinary].VirtualStart = kernelStart;
	bootData.MemoryLayout.SpecialLocations[SpecialMemoryLocation_KernelBinary].PhysicalStart = kernelStart;
	bootData.MemoryLayout.SpecialLocations[SpecialMemoryLocation_KernelBinary].ByteSize = pageCount * EFI_PAGE_SIZE;

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

KernelStartFunction LoadKernel(EFI_HANDLE imageHandle, EFI_BOOT_SERVICES* bootServices, KernelBootData& bootData)
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

	KernelStartFunction kernelMain = PrepareKernel(bootServices, kernelImageBuffer, bootData);

	ERROR_CHECK(bootServices->FreePool(kernelImageBuffer), u"Error freeing kernel image");

	return kernelMain;
}
