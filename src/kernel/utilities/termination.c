#include "common/types.h"

void halt(void)
{
    asm("cli");
    while(true)
    {
        asm("hlt");
    }
}
