#include <stdint.h>
#include <stddef.h>

#include "memory/memory.h"
#include "kernel/init/gdt.h"

static GDTEntry GDT[] =
{
    // Null - 0 index, 0x0 offset
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
    // Code (64bit) - 1 index, 0x8 offset
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
    // Data (64bit) - 2 index, 0x10 offset
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
    },
	// Code (32bit) - 3 index, 0x18 offset
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
        .LongModeCode = 0,
        .OpSize = 1, //Always zero when LongModeCode is set
        .Granularity = 1, //4KiB
        .BaseHigh = 0
    },
    // Data (64bit) - 4 index, 0x20 offset
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

static_assert(sizeof(GDTEntry) == 0x8);

void InitGDT(uint8_t* target)
{
	GDTPointer GDTLimits;

    GDTLimits.Limit = sizeof(GDT) - 1;
    GDTLimits.Base = (uint64_t)target;

	memcpy(target, GDT, sizeof(GDT));

    //Load the GDT
    asm volatile("lgdt %0" : : "m"(GDTLimits));

    //Force a jump to apply all the changes
    ReloadSegments();
}
