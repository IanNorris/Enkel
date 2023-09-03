#include "utilities/termination.h"
#include "kernel/console/console.h"
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
	
	ConsolePrint(u"Exiting\n");

	HaltPermanently();
}
