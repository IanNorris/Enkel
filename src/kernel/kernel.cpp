#include "utilities/termination.h"
#include "kernel/console/console.h"
#include "kernel/devices/keyboard.h"
#include "kernel/init/apic.h"
#include "kernel/init/bootload.h"
#include "kernel/init/init.h"
#include "kernel/init/pic.h"
#include "kernel/init/tls.h"
#include "kernel/init/msr.h"
#include "kernel/init/interrupts.h"
#include "kernel/texture/render.h"
#include "fs/fat/fat.h"
#include "memory/memory.h"
#include "memory/virtual.h"
#include "rpmalloc.h"
#include "../../assets/SplashLogo.h"
#include "kernel/init/acpi.h"
#include "kernel/user_mode/syscall.h"

#include "Protocol/DevicePath.h"
#include "kernel/devices/ahci/cdrom.h"

#include <ff.h>

void EnterUserModeTest();
void InitializeSyscalls();

void RunElf(const uint8_t* elfStart);

KernelBootData GBootData;

extern const char16_t* KernelBuildId;

CdRomDevice* GCDRomDevice;

#include "common/string.h"

//Define what a constructor is
typedef void (*StaticInitFunction)();

extern "C" StaticInitFunction CRTStaticInitStart;
extern "C" StaticInitFunction CRTStaticInitEnd;
extern "C" void CRTInit()
{
    for(StaticInitFunction* InitFunction = &CRTStaticInitStart; InitFunction != &CRTStaticInitEnd; InitFunction++)
	{
		(*InitFunction)();
	}
}

void FinalizeRuntimeServices()
{
	uint32_t runtimeEntries = 0;
	for (uint32_t entry = 0; entry < GBootData.MemoryLayout.Entries; entry++)
    {
        EFI_MEMORY_DESCRIPTOR& Desc = *((EFI_MEMORY_DESCRIPTOR*)((UINT8*)GBootData.MemoryLayout.Map + (entry * GBootData.MemoryLayout.DescriptorSize)));
		if(Desc.Attribute & EFI_MEMORY_RUNTIME)
		{
			runtimeEntries++;
		}
	}

	EFI_MEMORY_DESCRIPTOR* entries = (EFI_MEMORY_DESCRIPTOR*)rpmalloc(sizeof(EFI_MEMORY_DESCRIPTOR) * runtimeEntries);

	int runtimeEntry = 0;
	for (uint32_t entry = 0; entry < GBootData.MemoryLayout.Entries; entry++)
    {
        EFI_MEMORY_DESCRIPTOR& Desc = *((EFI_MEMORY_DESCRIPTOR*)((UINT8*)GBootData.MemoryLayout.Map + (entry * GBootData.MemoryLayout.DescriptorSize)));
	
		if(Desc.Attribute & EFI_MEMORY_RUNTIME)
		{
			entries[runtimeEntry] = Desc;
			runtimeEntry++;
		}
    }

	EFI_STATUS runtimeServicesAddressMapStatus = GBootData.RuntimeServices->SetVirtualAddressMap(sizeof(EFI_MEMORY_DESCRIPTOR) * runtimeEntries, sizeof(EFI_MEMORY_DESCRIPTOR), EFI_MEMORY_DESCRIPTOR_VERSION, entries);
	if(runtimeServicesAddressMapStatus != EFI_SUCCESS)
	{
		char16_t Buffer[16];
		ConsolePrint(u"SetVirtualAddressMap error: 0x");
		witoabuf(Buffer, (int)runtimeServicesAddressMapStatus, 16);
		ConsolePrint(Buffer);
		ConsolePrint(u"\r\n");

		_ASSERTF(false, "Unable to transition to virtual address map for runtime services.");
	}

	rpfree(entries);
}

KernelState NextTask;

FRESULT scan_files (
    char16_t* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;


    res = f_opendir(&dir, (const TCHAR*)path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
				strcat(&path[i], u"/");
				strcat(&path[i+1], (const char16_t*)fno.fname);
                //sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
				ConsolePrint(path);
				ConsolePrint(u"/");
				ConsolePrint((const char16_t*)fno.fname);
                ConsolePrint(u"\n");
            }
        }
        f_closedir(&dir);
    }

    return res;
}

void RunApp(const char16_t* appName)
{
	FIL file;

	ConsolePrint(u"Running ");
	ConsolePrint(appName);
	ConsolePrint(u"\n");

	FRESULT fr = f_open(&file, (const TCHAR*)appName, FA_READ);
	if(fr == FR_OK)
	{
		uint64_t fileSize = f_size(&file);

		uint64_t alignedSize = AlignSize(fileSize, 4096);

		uint8_t* buffer = (uint8_t*)VirtualAlloc(alignedSize,  PrivilegeLevel::User);
		UINT bytesRead;
		fr = f_read(&file, buffer, fileSize, &bytesRead);

		RunElf(buffer);

		VirtualFree(buffer, alignedSize);
	}
}

extern "C" void __attribute__((sysv_abi, __noreturn__)) KernelMain(KernelBootData * BootData)
{
	OnKernelMainHook();

	GBootData = *BootData;

	DefaultConsoleInit(GBootData.Framebuffer, BMFontColor{ 0x0, 0x20, 0x80 }, BMFontColor{ 0x8c, 0xFF, 0x0 });

	RenderTGA(&GBootData.Framebuffer, SplashLogo_tga_data, GBootData.Framebuffer.Width, 0, AlignImage::Right, AlignImage::Top, true);

	ConsolePrint(u"Starting Enkel (Revision ");
	ConsolePrint(KernelBuildId);
	ConsolePrint(u")...\n");

	if (IsDebuggerPresent())
	{
		ConsolePrint(u"DEBUGGER ATTACHED!\n");
	}

	InitVirtualMemory(&GBootData);

	ConsolePrint(u"Initializing CRT...\n");
	CRTInit();

	ConsolePrint(u"Initializing interrupts...\n");
	InitInterrupts(&GBootData);

	//FinalizeRuntimeServices();

	ConsolePrint(u"Initializing PIC...\n");
	InitPIC();

	ConsolePrint(u"Initializing APIC...\n");
	InitApic(GBootData.Rsdt, GBootData.Xsdt);

	ConsolePrint(u"Initializing keyboard...\n");
	InitKeyboard();

#ifdef _DEBUG
	ACPI_DEBUG_INITIALIZE ();
#endif

	ConsolePrint(u"Initializing syscalls...\n");
	InitializeSyscalls();

	ConsolePrint(u"Initializing ACPI...\n");
	InitializeAcpica();

	//WalkAcpiTree();

	EFI_DEV_PATH* devicePath = (EFI_DEV_PATH*)BootData->BootDevicePath;

	SataBus* sataBus = (SataBus*)rpmalloc(sizeof(SataBus));
	sataBus->Initialize(devicePath);

	CdRomDevice* cdromDevice = (CdRomDevice*)rpmalloc(sizeof(CdRomDevice));
	cdromDevice->Initialize(devicePath, sataBus);
	GCDRomDevice = cdromDevice;

	//uint64_t tocSize = 800;
	//uint8_t* tocBuffer = (uint8_t*)rpmalloc(tocSize);
	//cdromDevice->ReadToc(tocBuffer, tocSize);
	//HexDump((uint8_t*)tocBuffer, tocSize, 64);

	/*uint64_t fileSystemImageSize = 32 * 1024 * 1024;
	uint8_t* fileSystemImage = (uint8_t*)rpmalloc(fileSystemImageSize);

	int sectorSize = 2048;
	int startSector = 33; //(0x10800 / 2048 - which seems to be where our file system starts)
	int sectorCount = fileSystemImageSize / sectorSize;

	uint8_t* fileSystemImagePointer = fileSystemImage;
	int offsetSector = 0;
	do
	{
		int sectorCountToRead = sectorCount > 255 ? 255 : sectorCount;

		cdromDevice->ReadSectors(startSector + offsetSector, sectorCountToRead, fileSystemImagePointer);
		fileSystemImagePointer += sectorCountToRead * sectorSize;
		offsetSector += sectorCountToRead;
		
		sectorCount -= sectorCountToRead;
	} while(sectorCount > 0);*/

	FATFS fs;
    FRESULT res;
    char16_t buff[256];


    res = f_mount(&fs, (const TCHAR*)u"", 1);
    if (res == FR_OK) {
        strcpy(buff, u"/");
        res = scan_files(buff);
    }

	RunApp(u"//hello_world.a");

	//Crashes accessing FS[0x28]...
	RunApp(u"//count.a");

	ConsolePrint(u"Ready!\n");
	//HaltPermanently();

	while(true)
	{
		asm volatile("hlt");
	}
}
