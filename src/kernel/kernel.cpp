#include "utilities/termination.h"
#include "kernel/console/console.h"
#include "kernel/init/apic.h"
#include "kernel/init/bootload.h"
#include "kernel/init/init.h"
#include "kernel/init/interrupts.h"
#include "memory/memory.h"
#include "memory/virtual.h"

KernelBootData GBootData;

extern const char16_t* KernelBuildId;

#include "kernel/utilities/slow.h"
#include "common/string.h"

void WriteToMemoryWeOwn(void* Address, uint64_t Size, int Pattern)
{
	memset(Address, Pattern, Size);
}

void WriteToMemoryWeDontOwn(void* Address, uint64_t Size, int Pattern)
{
	memset(Address, Pattern, Size);
}

extern "C" void __attribute__((sysv_abi, __noreturn__)) KernelMain(KernelBootData * BootData)
{
	OnKernelMainHook();

	GBootData = *BootData;

	DefaultConsoleInit(GBootData.Framebuffer, BMFontColor{ 0x0, 0x20, 0x80 }, BMFontColor{ 0x8c, 0xFF, 0x0 });

	ConsolePrint(u"Starting Enkel (Revision ");
	ConsolePrint(KernelBuildId);
	ConsolePrint(u")...\n");

	if (IsDebuggerPresent())
	{
		ConsolePrint(u"DEBUGGER ATTACHED!\n");
	}

	EnterLongMode(&GBootData);

	//This still doesn't work...
	/*EFI_STATUS runtimeServicesAddressMapStatus = GBootData.RuntimeServices->SetVirtualAddressMap(GBootData.MemoryLayout.MapSize, GBootData.MemoryLayout.DescriptorSize, GBootData.MemoryLayout.DescriptorVersion, GBootData.MemoryLayout.Map);
	if(runtimeServicesAddressMapStatus != EFI_SUCCESS)
	{
		char16_t Buffer[16];
		ConsolePrint(u"SetVirtualAddressMap error: 0x");
		witoabuf(Buffer, (int)runtimeServicesAddressMapStatus, 16);
		ConsolePrint(Buffer);
		ConsolePrint(u"\r\n");

		_ASSERTF(false, "Unable to transition to virtual address map for runtime services.");
	}*/

	InitApic(GBootData.Xsdt);
	
	//ConsolePrint(u"Exiting\n");
	//HaltPermanently();

	while(true)
	{
		asm volatile("hlt");
	}
}
