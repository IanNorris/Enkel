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
#include "memory/memory.h"
#include "memory/virtual.h"
#include "rpmalloc.h"
#include "../../assets/SplashLogo.h"
#include "kernel/init/acpi.h"
#include "kernel/user_mode/syscall.h"

#include "Protocol/DevicePath.h"
#include "kernel/devices/ahci/cdrom.h"

void EnterUserModeTest();
void InitializeSyscalls();

KernelBootData GBootData;

extern const char16_t* KernelBuildId;

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

	EnterUserModeTest();

	InitializeAcpica();

	//WalkAcpiTree();

	EFI_DEV_PATH* devicePath = (EFI_DEV_PATH*)BootData->BootDevicePath;

	SataBus* sataBus = (SataBus*)rpmalloc(sizeof(SataBus));
	sataBus->Initialize(devicePath);

	CdRomDevice* cdromDevice = (CdRomDevice*)rpmalloc(sizeof(CdRomDevice));
	cdromDevice->Initialize(devicePath, sataBus);

	uint8_t buffer[2048];
	uint8_t buffer2[2048];

	cdromDevice->ReadSector(0, buffer);
	cdromDevice->ReadSector(1, buffer2);
	
	ConsolePrint(u"Ready!\n");
	//HaltPermanently();

	while(true)
	{
		asm volatile("hlt");
	}
}
