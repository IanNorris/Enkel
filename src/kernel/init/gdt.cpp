#include <stdint.h>
#include <stddef.h>

#include "memory/memory.h"
#include "kernel/init/gdt.h"

static GDTEntry GDT[] =
{
    // Null
    {
        .LimitLow = 0,
        .BaseLow = 0,
        .BaseMiddle = 0,
        .Accessed = 0,
        .ReadWrite = 0,
        .DirectionConforming = 0,
        .Executable = 0,
        .DescriptorType = 0,
        .Privilege = 0,
        .Present = 0,
        .LimitHigh = 0x0,
        .Reserved = 0,
        .LongModeCode = 0,
        .OpSize = 0,
        .Granularity = 0,
        .BaseHigh = 0
    },
    // Code
    {
        .LimitLow = 0xFFFF,
        .BaseLow = 0,
        .BaseMiddle = 0,
        .Accessed = 0,
        .ReadWrite = 1, //Can READ from code (can never write when Executable)
        .DirectionConforming = 0, //?
        .Executable = 1,
        .DescriptorType = 1,
        .Privilege = 0,
        .Present = 1,
        .LimitHigh = 0xF,
        .Reserved = 0,
        .LongModeCode = 1,
        .OpSize = 0, //Always zero when LongModeCode is set
        .Granularity = 1, //4KiB
        .BaseHigh = 0
    },
    // Data
    {
        .LimitLow = 0xFFFF,
        .BaseLow = 0,
        .BaseMiddle = 0,
        .Accessed = 0,
        .ReadWrite = 1, // Writeable data segment
        .DirectionConforming = 0,
        .Executable = 0,
        .DescriptorType = 1,
        .Privilege = 0,
        .Present = 1,
        .LimitHigh = 0xF,
        .Reserved = 0,
        .LongModeCode = 0,
        .OpSize = 1,
        .Granularity = 1, //4KiB
        .BaseHigh = 0
    }
};

static GDTPointer GDTLimits;

static_assert(sizeof(GDTEntry) == 0x8);

void InitGDT()
{
    GDTLimits.Limit = sizeof(GDT) - 1;
    GDTLimits.Base = (uint64_t)GDT;

    //Load the GDT
    SetGDT((sizeof(GDTEntry) * 3) - 1, (uint64_t)&GDT[0]);

    //Force a jump to apply all the changes
    ReloadSegments();
}
