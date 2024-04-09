#include <stdint.h>
#include <stddef.h>

#include "memory/memory.h"
#include "kernel/init/gdt.h"

extern "C"
{
	GDTPointer GDTLimits;
	TSS TSSRing0;
}

static GDTDescriptors GDT =
{
	/* README: The order here is very important deliberate.
	* When doing syscalls KernelCS must be set in STAR 47:32, (0x8)
	* and UM Data and UM Code are calculated as +8 and +16 from STAR 63:48 (so in this case we use 0x18)
	* See Intel SDM section 5.8.8 Fast System Calls in 64-Bit Mode
	*    When SYSRET transfers control to 64-bit mode user code using REX.W, the processor gets the privilege level 3
	*    target code segment, instruction pointer, stack segment, and flags as follows :
	*    * Target code segment - Reads a non-NULL selector from IA32_STAR[63:48] + 16.
	*/

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
		// Kernel Code (64bit) - 1 index, 0x8 offset
		{
			.LimitLow = 0xFFFF,
			.BaseLow = 0,
			.BaseMiddle = 0,
			.Accessed = 0,
			.ReadWrite = 1, //Can READ from code (can never write when Executable) (TODO CHECKME)
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
		// Kernel Data (64bit) - 2 index, 0x10 offset
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
		// Star 48 NULL - 3 index, 0x18 offset (can be 32bit UM code if required)
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
		// User Data (64bit) - 4 index, 0x20 offset
		{
			.LimitLow = 0xFFFF,
			.BaseLow = 0,
			.BaseMiddle = 0,
			.Accessed = 0,
			.ReadWrite = 1, // Writeable data segment
			.DirectionConforming = 0,
			.Executable = 0,
			.DescriptorType = 1,
			.Privilege = 3,
			.Present = 1,
			.LimitHigh = 0xF,
			.Reserved = 0,
			.LongModeCode = 0,
			.OpSize = 1,
			.Granularity = 1, //4KiB
			.BaseHigh = 0
		},
		// User Code (64bit) - 5 index, 0x28 offset
		{
			.LimitLow = 0xFFFF,
			.BaseLow = 0,
			.BaseMiddle = 0,
			.Accessed = 0,
			.ReadWrite = 1, //Can READ from code (can never write when Executable) (TODO CHECKME)
			.DirectionConforming = 0, //?
			.Executable = 1,
			.DescriptorType = 1,
			.Privilege = 3,
			.Present = 1,
			.LimitHigh = 0xF,
			.Reserved = 0,
			.LongModeCode = 1,
			.OpSize = 0, //Always zero when LongModeCode is set
			.Granularity = 1, //4KiB
			.BaseHigh = 0
		}
	},

	//TSS
	{
		.LimitLow = 0xFFFF,
		.BaseLow = 0,
		.BaseMiddle = 0,
		.AccessByte = 0x89, //Present, Executable, Accessed
		.Flags = 0x0,
        .BaseHigh = 0,
		.BaseUpper = 0,
		.Reserved = 0,
	}
};

static_assert(sizeof(GDTEntry) == 0x8);
static_assert(sizeof(TSSEntry) == 0x10);
static_assert(sizeof(TSS) == 104);

void InitGDT(uint8_t* target)
{
	memset(&TSSRing0, 0, sizeof(TSS));
	TSSRing0.IoMapBaseAddress = sizeof(TSS);

	uint64_t tssBase = (uint64_t)&TSSRing0;
	GDT.TSS.LimitLow = sizeof(TSS);
	GDT.TSS.AccessByte = 0x89; //Present, Executable, Accessed
	GDT.TSS.Flags = 0x0;
	GDT.TSS.BaseLow = tssBase & 0xFFFF;
	GDT.TSS.BaseMiddle = (tssBase >> 16) & 0xFF;
	GDT.TSS.BaseHigh = (tssBase >> 24) & 0xFF;
	GDT.TSS.BaseUpper = (tssBase >> 32);

    GDTLimits.Limit = sizeof(GDT) - 1;
    GDTLimits.Base = (uint64_t)target;

	memcpy(target, &GDT, sizeof(GDT));

    //Load the GDT
    asm volatile("lgdt %0" : : "m"(GDTLimits));

    //Force a jump to apply all the changes
    ReloadSegments();
}
