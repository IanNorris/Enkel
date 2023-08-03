#include "kernel/init/gdt.h"
#include "kernel/init/interrupts.h"

void EnterLongMode()
{
    //Disable interrupts because we won't be able to handle these until we initialize the GDT and the interrupt handlers.
    DisableInterrupts();

    InitGDT();
    InitIDT();

    //We can now re-enable interrupts.
    EnableInterrupts();
}
