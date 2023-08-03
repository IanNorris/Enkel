#include "utilities/termination.h"

#include "kernel/console/console.h"
#include "kernel/init/bootload.h"
#include "kernel/init/init.h"

KernelBootData GBootData;

extern "C" void __attribute__((sysv_abi, __noreturn__)) KernelMain(KernelBootData* BootData)
{
    GBootData = *BootData;

    DefaultConsoleInit(GBootData.Framebuffer, BMFontColor{ 0x0, 0x20, 0x80 }, BMFontColor{ 0x8c, 0xFF, 0x0 });

    ConsolePrint(u"Starting Enkel...\n");

    ConsolePrint(u"Entering long mode...\n");
    EnterLongMode();

    asm volatile("int $3");

    ConsolePrint(u"Exiting\n");

    HaltPermanently();
}
