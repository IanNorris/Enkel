#include "helpers.h"
#include "memory/memory.h"
#include "bootloader_build.h"
#include "fs/fat/fat.h"

//void load_image(CEfiSystemTable* systemTable, void* imageBase);

CEfiStatus efi_main(CEfiHandle imageHandle, CEfiSystemTable* systemTable)
{
	systemTable->con_out->reset(systemTable->con_out, 0);
	InitHelpers(systemTable);

	systemTable->con_out->set_attribute(systemTable->con_out, C_EFI_LIGHTGREEN);
	Print(u"Starting Enkel (Revision ");
	Print(BOOTLOADER_BUILD_ID);
	Print(u")...\r\n");
	systemTable->con_out->set_attribute(systemTable->con_out, C_EFI_WHITE);

	//Set a timeout of 5s, if we haven't transferred to the kernel by then we probably won't be...
	CEfiBootServices* bootServices = systemTable->boot_services;
	bootServices->set_watchdog_timer(5, 0, 0, 0);

	Print(u"1");
	
	CEfiGuid loadedImageServiceGuid = C_EFI_LOADED_IMAGE_PROTOCOL_GUID;
	CEfiLoadedImageProtocol* loadedImageService;
	ERROR_CHECK(bootServices->handle_protocol(imageHandle , &loadedImageServiceGuid, (void**)&loadedImageService), u"LI service");

	//load_image(systemTable, loadedImageService->image_base);
	
	
	

	Print(u"\r\n");
		
	Print(u"2");

	Print(u"3");

	while (1)
	{
		Print(u".");
		bootServices->stall(ONE_SECOND);
	}

	return 0;
}

void load_image(CEfiSystemTable* systemTable, void* imageBase)
{
	//mem_open();

	//bootImage = imageBase;

/*	// Initialize the FS
	FAT16 fat;
	ff_init(&memoryDevice, &fat);

	FFILE file;
	ff_root(&fat, &file);

	char str[12];
	CEfiChar16 buffer[64];

	do
	{
		if (!ff_is_regular(&file))
			continue;

		ff_dispname(&file, str);
		ascii_to_wide(buffer, str, sizeof(buffer));

		Print(buffer);

	} while (ff_next(&file));
	*/
}
