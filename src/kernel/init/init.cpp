#include "kernel/init/bootload.h"
#include "kernel/init/gdt.h"
#include "kernel/init/interrupts.h"
#include "kernel/init/long_mode.h"
#include "kernel/memory/pml4.h"
#include "kernel/console/console.h"
#include "common/string.h"

void EnterLongMode(KernelBootData* bootData)
{
    //Disable interrupts because we won't be able to handle these until we initialize the GDT and the interrupt handlers.
    DisableInterrupts();

    ConsolePrint(u"Initializing GDT...\n");
    InitGDT();

    ConsolePrint(u"Initializing IDT...\n");
    InitIDT();

    //We can now re-enable interrupts.
    EnableInterrupts();

    _ASSERTF(CheckProtectedMode(), "In UEFI mode we should already be in protected mode!");

    //We don't need to disable paging or set PAE mode because we're already in long mode
    //thanks to being booted via EFI. So We just need to give our new PML4 table to the CPU.

    BuildAndLoadPML4(bootData);
}
