#include "kernel/init/bootload.h"
#include "kernel/init/apic.h"
#include "kernel/init/msr.h"
#include "kernel/console/console.h"
#include "common/string.h"
#include "memory/physical.h"
#include "kernel/scheduling/time.h"
#include "kernel/utilities/codegen.h"

#include "Protocol/AcpiSystemDescriptionTable.h"

struct EFI_ACPI_HIGH_PRECISION_EVENT_TIMER_TABLE_HEADER;
void InitHPET(EFI_ACPI_HIGH_PRECISION_EVENT_TIMER_TABLE_HEADER* Header);

extern KernelBootData GBootData;

const uint32_t IA32_APIC_BASE_MSR = 0x1B;
const uint32_t MSR_BSP_MASK = 0x100;
const uint32_t APIC_ENABLE = 0x100;
const uint32_t MSR_ENABLE_MASK = 0x800;

const uint32_t  DELIVERY_MODE_INIT = 0x500;
const uint32_t  DELIVERY_MODE_STARTUP = 0x600;

const uint32_t LEVEL_TRIGGER = 0x8000;
const uint32_t LEVEL_ASSERT = 0x4000;

const uint16_t PIC1_COMMAND = 0x20; // IO base address for master PIC
const uint16_t PIC1_DATA = 0x21;
const uint16_t PIC2_COMMAND = 0xA0; // IO base address for slave PIC
const uint16_t PIC2_DATA = 0xA1;

// Assuming the startup code is located at 0x1000 in real mode.
// This is just an example; adjust as necessary.
const uint32_t  STARTUP_ADDRESS_SEGMENT = 0x1000;

uint64_t LocalApicPhysical;
volatile uint8_t* LocalApicVirtual;

const unsigned int MaxProcessors = 256;
uint8_t ProcessorIds[MaxProcessors];
unsigned int BSPId;
unsigned int ProcessorCount;

uint64_t APStackHigh = 0x0;
volatile uint64_t APSignal = 0x0;

extern uint64_t PML4;
extern uint64_t GDTRegister;
extern uint64_t APEntry;
extern uint64_t APEntryEnd;
extern uint64_t IDTLimits;

extern "C" void __attribute__((naked, used, noreturn)) APEntryFunction()
{
	APSignal = true;

	ConsolePrint(u"CORE ONLINE!\n");

	while(true)
	{
		asm("pause");
	}
}

void WriteLocalApic(uint32_t Offset, uint32_t Value, uint32_t Mask)
{
	uint32_t Existing = *(volatile uint32_t*)(LocalApicVirtual+Offset);
	uint32_t NewValue = (Existing & ~Mask) | (Value & Mask);
	*(volatile uint32_t*)(LocalApicVirtual+Offset) = NewValue;

	char16_t Buffer[16];

	ConsolePrint(u"Wrote to 0x");
	witoabuf(Buffer, Offset, 16);
	ConsolePrint(Buffer);
	ConsolePrint(u" value 0x");
	witoabuf(Buffer, NewValue, 16);
	ConsolePrint(Buffer);
	ConsolePrint(u"\n");
}

uint32_t ReadLocalApic(uint32_t Offset)
{
	uint32_t Result = *(volatile uint32_t*)(LocalApicVirtual+Offset);

	char16_t Buffer[16];

	ConsolePrint(u"Read from 0x");
	witoabuf(Buffer, Offset, 16);
	ConsolePrint(Buffer);
	ConsolePrint(u" value 0x");
	witoabuf(Buffer, Result, 16);
	ConsolePrint(Buffer);
	ConsolePrint(u"\n");

	return Result;
}

void WaitForIdleIPI()
{
	uint32_t ErrorStatus;
	uint32_t APSelector;
	bool IsFinished;
	do
	{
		asm volatile("pause");
		APSelector = ReadLocalApic((uint32_t)LocalApicOffsets::InterruptCommandRegisterLow);
		IsFinished = (APSelector & (1 << 12)) == 0;

		ErrorStatus = ReadLocalApic((uint32_t)LocalApicOffsets::ErrorStatusRegister);
		if(ErrorStatus != 0)
		{
			char16_t Buffer[16];

			ConsolePrint(u"Error 0x");
			witoabuf(Buffer, ErrorStatus, 16);
			ConsolePrint(Buffer);
		ConsolePrint(u"\n");
		}
		_ASSERTF(ErrorStatus == 0, "APIC error");
	} while(!IsFinished);
}

void InitAPs()
{
	char16_t Buffer[16];

	const int APStackSize = 256 * 1024;
	uint64_t APStack = (uint64_t)VirtualAlloc(APStackSize);
	APStackHigh = APStack + APStackSize;

	uint8_t* APTrampoline = (uint8_t*)GBootData.MemoryLayout.SpecialLocations[SpecialMemoryLocation_APBootstrap].VirtualStart;
	
	ProcessorCount = 2;
    for(unsigned int processor = 0; processor < ProcessorCount; processor++)
	{
		if(ProcessorIds[processor] == BSPId)
		{
			continue;
		}

		uint8_t* APTrampolinePtr = APTrampoline;

		VirtualProtect(APTrampoline, PAGE_SIZE, MemoryProtection::ReadWrite);

		//Stop on init
		*APTrampolinePtr++ = 0xFA; //cli
		*APTrampolinePtr++ = 0xF4; //hlt

		//Configure the stack
		APTrampolinePtr = Write_MovRelativeToRegister(APTrampolinePtr, Register::rbp, (uint8_t*)&APStackHigh);
		APTrampolinePtr = Write_MovRelativeToRegister(APTrampolinePtr, Register::rsp, (uint8_t*)&APStackHigh);

		//Configure registers to read from relative addresses
		APTrampolinePtr = Write_MovRelativeToRegister(APTrampolinePtr, Register::rdi, (uint8_t*)&PML4);
		APTrampolinePtr = Write_MovRelativeToRegister(APTrampolinePtr, Register::rdx, (uint8_t*)&GDTRegister);
		APTrampolinePtr = Write_MovRelativeToRegister(APTrampolinePtr, Register::rcx, (uint8_t*)&APEntryFunction);
		APTrampolinePtr = Write_MovRelativeToRegister(APTrampolinePtr, Register::r8, (uint8_t*)&IDTLimits);

		//Now copy the whole of APEntry
		uint64_t APEntrySize = (uint64_t)&APEntryEnd - (uint64_t)&APEntry;
		memcpy(APTrampolinePtr, &APEntry, APEntrySize);
		APTrampolinePtr += APEntrySize;

		VirtualProtect(APTrampoline, PAGE_SIZE, MemoryProtection::Execute);

		uint64_t StartupAddress = GetPhysicalAddress((uint64_t)APTrampoline);
		StartupAddress = StartupAddress >> 12;
		uint64_t StartupAddressTemp = StartupAddress;
		StartupAddress &= 0xFF;

		_ASSERTF(StartupAddress == StartupAddressTemp, "Misaligned AP startup vector");

		//IMPORTANT NOTE
		// If you're debugging this in an emulator, these values are not going to look right in the debugger
		// because the debugger skips the path that intercepts these loads.
		//IMPORTANT NOTE

		WriteLocalApic( (uint32_t)LocalApicOffsets::ErrorStatusRegister, 0, ~0);

		// Upper ICR: set the target processor's APIC ID and send INIT IPI
		WriteLocalApic( (uint32_t)LocalApicOffsets::InterruptCommandRegisterHigh, ProcessorIds[processor] << 24, 0xFF << 24);
		WriteLocalApic( (uint32_t)LocalApicOffsets::InterruptCommandRegisterLow, LEVEL_TRIGGER | LEVEL_ASSERT | DELIVERY_MODE_INIT, 0xFFFFF);

		ReadLocalApic( (uint32_t)LocalApicOffsets::InterruptCommandRegisterLow );

		HpetSleepMS(10);

		WaitForIdleIPI();

		// Send the Startup IPI twice as recommended in some Intel manuals.
		APSignal = 0;
		for(int i = 0; i < 2; i++)
		{
			WriteLocalApic( (uint32_t)LocalApicOffsets::ErrorStatusRegister, 0, ~0);

			WriteLocalApic( (uint32_t)LocalApicOffsets::InterruptCommandRegisterHigh, ProcessorIds[processor] << 24, 0xFF << 24);
			WriteLocalApic( (uint32_t)LocalApicOffsets::InterruptCommandRegisterLow, DELIVERY_MODE_STARTUP | StartupAddress, ~0xfff0f800);
			
			HpetSleepMS(200);
			WaitForIdleIPI();
		}

		ReadLocalApic( (uint32_t)LocalApicOffsets::InterruptCommandRegisterLow );

		while(APSignal == 0)
		{
			asm volatile("pause");
		}
    }
}

void InitMADT(EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER* MADT)
{
	LocalApicPhysical = (uint64_t)MADT->LocalApicAddress; //Physical address

	bool LegacyPICs = MADT->Flags & 0x1;

	char16_t Buffer[16];

	ProcessorCount = 0;

	int LengthRemaining = MADT->Header.Length - sizeof(EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER);
	uint8_t* Data = (uint8_t*)(MADT+1);
	while(LengthRemaining > 0)
	{
		uint8_t Type = *Data;
		uint8_t Length = *(Data+1);

		switch(Type)
		{
			case EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC:
				{
					EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC_STRUCTURE* LocalAPIC = (EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC_STRUCTURE*)Data;

					ProcessorIds[ProcessorCount] = LocalAPIC->ApicId;
					ProcessorCount++;

					break;
				}

			case EFI_ACPI_2_0_IO_APIC:
				{
					ConsolePrint(u"Found IO APIC\n");
					break;
				}

			case EFI_ACPI_2_0_LOCAL_APIC_ADDRESS_OVERRIDE:
				{
					EFI_ACPI_2_0_LOCAL_APIC_ADDRESS_OVERRIDE_STRUCTURE* LocalAPICAddressOverride = (EFI_ACPI_2_0_LOCAL_APIC_ADDRESS_OVERRIDE_STRUCTURE*)Data;
					LocalApicPhysical = LocalAPICAddressOverride->LocalApicAddress;

					ConsolePrint(u"Found Local APIC address override\n");
					break;
				}
		}

		LengthRemaining -= Length;
		Data += Length;
	}

	ConsolePrint(u"Found ");
	witoabuf(Buffer, ProcessorCount, 10);
	ConsolePrint(Buffer);
	ConsolePrint(u" cores.\n");

	LocalApicVirtual = (uint8_t*)PhysicalAlloc((uint64_t)LocalApicPhysical, 4096, (PageFlags)(PageFlags_Cache_Disable | PageFlags_Cache_WriteThrough));

	//Enable APIC
	uint64_t msr = GetMSR(IA32_APIC_BASE_MSR);
	msr = (msr & ~0xFFF) | 0x800; //Global APIC enable bit
	SetMSR(IA32_APIC_BASE_MSR, msr);

	msr = GetMSR(IA32_APIC_BASE_MSR);
	if(!(msr & 0x800))
	{
		_ASSERTF(false, "APIC not enabled");
	}
	

	BSPId = ReadLocalApic((uint32_t)LocalApicOffsets::LocalApicIdRegister);

	WriteLocalApic( (uint32_t)LocalApicOffsets::SpuriousInterruptVectorRegister, APIC_ENABLE | 0xFF, 0x1FF);

	_ASSERTF(LengthRemaining == 0, "Overshot MADT end");

	InitAPs();
}

void InitApic(EFI_ACPI_DESCRIPTION_HEADER* Xsdt)
{
	EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER* MADT = nullptr;
	EFI_ACPI_HIGH_PRECISION_EVENT_TIMER_TABLE_HEADER* HPET = nullptr;

	int XsdtEntries = (Xsdt->Length - sizeof(EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(EFI_ACPI_DESCRIPTION_HEADER*); //Take off the header and each entry is 8b
	for(int XsdtEntry = 0; XsdtEntry < XsdtEntries; XsdtEntry++)
	{
		EFI_ACPI_DESCRIPTION_HEADER* Header = ((EFI_ACPI_DESCRIPTION_HEADER**)(Xsdt+1))[XsdtEntry];

		//MADT
		if(Header->Signature == EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE)
		{
			MADT = (EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER*)Header;
			
		}
		//HPET
		else if(Header->Signature == EFI_ACPI_3_0_HIGH_PRECISION_EVENT_TIMER_TABLE_SIGNATURE)
		{
			HPET = (EFI_ACPI_HIGH_PRECISION_EVENT_TIMER_TABLE_HEADER*)Header;
		}
	}

	_ASSERTF(HPET, "HPET was not found in XSDT");
	InitHPET(HPET);

	_ASSERTF(MADT, "MADT was not found in XSDT");
	InitMADT(MADT);
}