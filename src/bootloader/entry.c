#include "helpers.h"
#include "memory/memory.h"
#include "bootloader_build.h"
//#include "fat16.h"

//void load_image(CEfiSystemTable* systemTable, void* imageBase);

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

	//load_image(systemTable, loadedImageService->image_base);
	
	
	

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

/*
BLOCKDEV memoryDevice;
uint32_t memoryPosition;
void* bootImage;


void mem_seek(const uint32_t pos)
{
	memoryPosition = pos;
}

void mem_rseek(const int16_t pos)
{
	memoryPosition += pos;
}

void mem_load(void* dest, const uint16_t len)
{
	memcpy(dest, ((char*)bootImage) + memoryPosition, len);
	memoryPosition += len;
}

void mem_store(const void* source, const uint16_t len)
{
}

void mem_write(const uint8_t b)
{
}

uint8_t mem_read()
{
	return *(((char*)bootImage) + memoryPosition++);
}

void mem_open()
{
	memoryDevice.read = &mem_read;
	memoryDevice.write = &mem_write;

	memoryDevice.load = &mem_load;
	memoryDevice.store = &mem_store;

	memoryDevice.seek = &mem_seek;
	memoryDevice.rseek = &mem_rseek;
}

void mem_close()
{
	bootImage = 0;
	memoryPosition = 0;
}
*/
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

		Print(systemTable, buffer);

	} while (ff_next(&file));
	*/
}