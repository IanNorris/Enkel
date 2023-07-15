#include "helpers.h"
#include "bootloader_build.h"

CEfiStatus efi_main(CEfiHandle imageHandle, CEfiSystemTable* systemTable)
{
	systemTable->con_out->reset(systemTable->con_out, 0);

	systemTable->con_out->set_attribute(systemTable->con_out, C_EFI_LIGHTGREEN);
	Print(systemTable, L"Starting Enkel (Revision ");
	Print(systemTable, BOOTLOADER_BUILD_ID);
	Print(systemTable, L")...\r\n");
	systemTable->con_out->set_attribute(systemTable->con_out, C_EFI_WHITE);

	//Set a timeout of 5s, if we haven't transferred to the kernel by then we probably won't be...
	CEfiBootServices* bootServices = systemTable->boot_services;
	bootServices->set_watchdog_timer(5, 0, 0, 0);

	Print(systemTable, L"1");
	
	CEfiGuid loadedImageServiceGuid = C_EFI_LOADED_IMAGE_PROTOCOL_GUID;
	CEfiLoadedImageProtocol* loadedImageService;
	ERROR_CHECK(bootServices->handle_protocol(imageHandle , &loadedImageServiceGuid, (void**)&loadedImageService), L"LI service");

	CEfiChar16 buffer[16];
	
	Print(systemTable, L"\r\n\r\n");

	for (int byte = 0; byte < 128; byte += 16)
	{
		for (int byteOffset = 0; byteOffset < 16; byteOffset++)
		{
			witoabuf(buffer, ((unsigned char*)loadedImageService->image_base)[byte + byteOffset], 16);

			Print(systemTable, buffer);
			Print(systemTable, L" ");
		}
		Print(systemTable, L"\r\n");
	}

	Print(systemTable, L"\r\n");
		
	
	Print(systemTable, L"2");

	Print(systemTable, L"3");

	while (1)
	{
		Print(systemTable, L".");
		bootServices->stall(ONE_SECOND);
	}

	return 0;
}
