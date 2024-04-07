#include "kernel/scheduling/time.h"
#include "kernel/init/msr.h"
#include "kernel/init/interrupts.h"
#include "kernel/console/console.h"
#include "common/string.h"
#include "memory/physical.h"

#include "kernel/init/bootload.h"
#include "IndustryStandard/HighPrecisionEventTimerTable.h"

volatile uint64_t* HpetBase;
volatile uint64_t* HpetCounter;
uint64_t HpetTicksPerNanosecond;

extern void WaitForPIT(uint64_t microseconds);

uint64_t GetCalibrationHPETSampleNS(bool warmUp)
{
	uint64_t DelayInMS = warmUp ? 1 : 50;

	uint64_t Start = *HpetCounter;

	WaitForPIT(DelayInMS * 1000);

	uint64_t End = *HpetCounter;

	uint64_t SecondFractions = 1000 / DelayInMS;

	uint64_t TicksPassed = End - Start;
	return (TicksPassed * SecondFractions);
}

void InitHPET(EFI_ACPI_HIGH_PRECISION_EVENT_TIMER_TABLE_HEADER* Header)
{
	HpetBase = (volatile uint64_t*)PhysicalAlloc(Header->BaseAddressLower32Bit.Address, 4096, PrivilegeLevel::Kernel);

	//Enable HPET
	uint64_t base = (uint64_t)HpetBase;
	volatile uint64_t* generalConfig = (volatile uint64_t*)(base + 0x10);
	*generalConfig |= 0x1;

	VerboseLog(u"Calibrating HPET...\n");

	HpetCounter = (volatile uint64_t*)(base + 0xF0);

	const int SampleCount = 6;
	uint64_t Total = 0;

	uint64_t Samples[SampleCount];

	//char16_t Buffer[16];
	//ConsolePrint(u"HPET samples: ");

	//Take SampleCount+1 samples, throwing the first one away
	for(int sample = 0; sample <= SampleCount; sample++)
	{
		uint64_t sampleValue = GetCalibrationHPETSampleNS(sample == 0);

		//witoabuf(Buffer, (int)sampleValue, 10);
		//ConsolePrint(Buffer);
		//ConsolePrint(u", ");

		//Disregard the first sample in case something wasn't in cache.
		if(sample != 0)
		{
			Samples[sample-1] = sampleValue;
		}
	}

	uint64_t Lowest = ~0;
	uint64_t Highest = 0;

	//Identify min and max values
	for(int sample = 0; sample < SampleCount; sample++)
	{
		if(Samples[sample] < Lowest)
		{
			Lowest = Samples[sample];
		}

		if(Samples[sample] > Highest)
		{
			Highest = Samples[sample];
		}
	}


	uint64_t SamplesUsed = 0;
	for(int sample = 0; sample < SampleCount; sample++)
	{
		if(Samples[sample] == Lowest)
		{
			Lowest = ~0;
		}
		else if(Samples[sample] == Highest)
		{
			Highest = ~0;
		}
		else
		{
			Total += Samples[sample];
			SamplesUsed++;
		}
	}

	//ConsolePrint(u"\n");

	Total /= SamplesUsed;
	HpetTicksPerNanosecond = Total;

	//ConsolePrint(u"HPET avg t/ns: ");
	//witoabuf(Buffer, (int)HpetTicksPerNanosecond, 10);
	//ConsolePrint(Buffer);
	//ConsolePrint(u"\n");
}

void HpetSleepNS(uint64_t nanoseconds)
{
	uint64_t Start = *HpetCounter;
	uint64_t End = Start + (uint64_t)(nanoseconds * HpetTicksPerNanosecond) / 1000;

	while (*HpetCounter < End)
	{
		asm volatile("pause");
	}
}

#define NOP_10 asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop");
#define NOP_100 NOP_10 NOP_10 NOP_10 NOP_10 NOP_10 NOP_10 NOP_10 NOP_10 NOP_10 NOP_10
#define NOP_1000 NOP_100 NOP_100 NOP_100 NOP_100 NOP_100 NOP_100 NOP_100 NOP_100 NOP_100

void HpetSleepUS(uint64_t microseconds)
{
	uint64_t Start = *HpetCounter;
	uint64_t End = Start + (uint64_t)(microseconds * HpetTicksPerNanosecond) / (1000 * 1000);

	while(*HpetCounter < End)
	{
		NOP_1000
	}
}

void HpetSleepMS(uint64_t milliseconds)
{
	HpetSleepUS(1000 * milliseconds);
}

volatile uint64_t PITReceived = false;

DEFINE_NAMED_INTERRUPT(PITInterrupt)(uint64_t interruptNumber, uint64_t rip, uint64_t cr2, uint64_t errorCode, uint64_t codeSegment, uint64_t triggeringRBP)
{
	PITReceived = 1;

	OutPort(0x20, 0x20);
}

void WaitForPIT(uint64_t microseconds)
{
	// Frequency of PIT
    uint32_t frequency = 1193182;  // 1.193182 MHz
    
    // The count value for 1 second
    uint64_t countValue = (microseconds * frequency) / 1000000;

	while(countValue > 0)
	{
		uint16_t actualCount = countValue > 65535 ? 65535 : countValue;
		// Send command byte
		OutPort(0x43, (0 << 6) | (3 << 4) | (3 << 1) | 0 );
		
		// Send count value
		OutPort(0x40, (uint8_t)(actualCount & 0xFF));        // Low byte
		OutPort(0x40, (uint8_t)((actualCount >> 8) & 0xFF)); // High byte

		PITReceived = 0;

		while(!PITReceived)
		{
			asm volatile("hlt");
		}

		countValue -= actualCount;
	}
}
