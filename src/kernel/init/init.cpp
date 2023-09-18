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
	//NOTE: We can't debug between these because the selector won't be valid yet
    DisableInterrupts();

	unsigned int codeSelector = 1;

	//Construct our IDT into the tables block
    ConsolePrint(u"Initializing IDT...\n");
	uint8_t* TableOffset = (uint8_t*)bootData->MemoryLayout.SpecialLocations[SpecialMemoryLocation_Tables].VirtualStart;
    size_t IDTTableSize = InitIDT(TableOffset, codeSelector);
	IDTTableSize = (IDTTableSize + 64) & 63;

	TableOffset += IDTTableSize;

	ConsolePrint(u"Initializing GDT...\n");
    InitGDT(TableOffset);

    //We can now re-enable interrupts.
    EnableInterrupts();

    _ASSERTF(CheckProtectedMode(), "In UEFI mode we should already be in protected mode!");

    //We don't need to disable paging or set PAE mode because we're already in long mode
    //thanks to being booted via EFI. So We just need to give our new PML4 table to the CPU.

    BuildAndLoadPML4(bootData);
}
