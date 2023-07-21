#include "helpers.h"
#include "memory/memory.h"
#include "common/string.h"
#include "bootloader_build.h"

#include "Protocol/LoadedImage.h"
#include "Protocol/GraphicsOutput.h"
#include "Guid/FileInfo.h"

void load_image(EFI_HANDLE imageHandle, EFI_BOOT_SERVICES* bootServices);

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
	systemTable->ConOut->Reset(systemTable->ConOut, 0);
	InitHelpers(systemTable);

	systemTable->ConOut->SetAttribute(systemTable->ConOut, EFI_LIGHTGREEN);
	Print(u"Starting Enkel (Revision ");
	Print(BOOTLOADER_BUILD_ID);
	Print(u")...\r\n");
	systemTable->ConOut->SetAttribute(systemTable->ConOut, EFI_WHITE);

	//Set a timeout of 15s, if we haven't transferred to the kernel by then we probably won't be...
	EFI_BOOT_SERVICES* bootServices = systemTable->BootServices;
	bootServices->SetWatchdogTimer(15, 0, 0, 0);

	load_image(imageHandle, bootServices);

	EFI_GRAPHICS_OUTPUT_PROTOCOL* GraphicsOutput;
	EFI_STATUS Status;

	// Get the Graphics Output Protocol instance
	EFI_GUID graphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	Status = bootServices->LocateProtocol(&graphicsOutputProtocolGuid, NULL, (VOID**)&GraphicsOutput);
	if (EFI_ERROR(Status)) {
		Print(u"Failed to locate Graphics Output Protocol\n");
		return Status;
	}

	// Get the mode information
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info = GraphicsOutput->Mode->Info;

	ERROR_CHECK(GraphicsOutput->SetMode(GraphicsOutput, 23), u"Setting graphics mode");

	// Get the frame buffer base
	EFI_PHYSICAL_ADDRESS  FrameBufferBase = GraphicsOutput->Mode->FrameBufferBase;
	*(uint32_t*)FrameBufferBase = 0xFF00FF00;

	// Now you can use FrameBufferBase and Info to directly write to the screen
	// Remember to call ExitBootServices when you're done using the boot services

	/*Status = gBS->ExitBootServices(imageHandle, MapTable);
	if (EFI_ERROR(Status)) {
		Print(u"Failed to exit boot services\n");
		return Status;
	}*/

	
	while (1)
	{
		Print(u".");
		bootServices->Stall(ONE_SECOND);
	}

	return 0;
}

void load_image(EFI_HANDLE imageHandle, EFI_BOOT_SERVICES* bootServices)
{
	EFI_GUID loadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	EFI_GUID simpleFileSystemProtocolGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
	EFI_GUID getInfoGuid = EFI_FILE_INFO_ID;

	EFI_LOADED_IMAGE_PROTOCOL* loadedImageService;
	ERROR_CHECK(bootServices->HandleProtocol(imageHandle, &loadedImageProtocolGuid, (void**)&loadedImageService), u"LI service");

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* simpleFileSystem;
	ERROR_CHECK(bootServices->OpenProtocol(loadedImageService->DeviceHandle, &simpleFileSystemProtocolGuid, (void**)&simpleFileSystem, imageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL), u"SFS protocol");

	EFI_FILE_PROTOCOL* volume;
	ERROR_CHECK(simpleFileSystem->OpenVolume(simpleFileSystem, &volume), u"Opening volume");

	EFI_FILE_PROTOCOL* kernelImage;
	ERROR_CHECK(volume->Open(volume, &kernelImage, (unsigned short*)u"KERNEL\\ENKEL.ELF", EFI_FILE_MODE_READ, 0), u"Opening kernal image");

	char Buffer[128];
	EFI_FILE_INFO* kernelFileInfo = (EFI_FILE_INFO*)Buffer;
	UINTN kernelInfoSize = sizeof(Buffer);

	ERROR_CHECK(kernelImage->GetInfo(kernelImage, &getInfoGuid, &kernelInfoSize, Buffer), u"Getting kernel file info");

	Print(u"Kernel is ");

	char16_t sizeBuffer[16];
	witoabuf(sizeBuffer, kernelFileInfo->FileSize / 1024, 10);

	Print(sizeBuffer);
	Print(u"KB.\r\n");

	void* kernelImageBuffer;
	UINTN outputBufferSize = kernelFileInfo->FileSize;

	ERROR_CHECK(bootServices->AllocatePool(EfiLoaderData, outputBufferSize, &kernelImageBuffer), u"Allocating kernel image buffer");

	ERROR_CHECK(kernelImage->Read(kernelImage, &outputBufferSize, kernelImageBuffer), u"Writing kernel into memory");
}
