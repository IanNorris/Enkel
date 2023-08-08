#include <stdint.h>
#include "kernel/utilities/slow.h"

#define NOP_SLED asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;")
#define NOP_SLED_X10 NOP_SLED; NOP_SLED; NOP_SLED; NOP_SLED; NOP_SLED; NOP_SLED; NOP_SLED; NOP_SLED; NOP_SLED; NOP_SLED;
#define NOP_SLEDX100 NOP_SLED_X10 NOP_SLED_X10 NOP_SLED_X10 NOP_SLED_X10 NOP_SLED_X10 NOP_SLED_X10 NOP_SLED_X10 NOP_SLED_X10 NOP_SLED_X10 NOP_SLED_X10

void Slow(SlowType type)
{
    uint64_t nopCount = 1000;

    switch (type)
    {
        case Flush:
            break;
        case Blip:
            nopCount *= 100;
            break;
        case Dawdle:
            nopCount *= 1000;
            break;
        case Loitering:
            nopCount *= 5000;
            break;
        case LoooooooongCat:
            nopCount *= 25000;
            break;
        case LoooooooongLoooooooongMan:
            nopCount *= 100000;
            break;
    }

    for (uint64_t index = 0; index < nopCount; index++)
    {
        NOP_SLEDX100
    }
}
