#include <stdint.h>
#include <stddef.h>

#include "memory/memory.h"
#include "utilities/termination.h"

#include "kernel/console/console.h"
#include "kernel/init/gdt.h"
#include "kernel/init/interrupts.h"
#include "kernel/init/msr.h"
#include "common/string.h"

#define IDT_SIZE 256

#define PAGE_FAULT_PRESENT (1 << 0)
#define PAGE_FAULT_WRITE (1 << 1)
#define PAGE_FAULT_USER (1 << 2)
#define PAGE_FAULT_RESERVED_WRITE (1 << 3)
#define PAGE_FAULT_EXECUTE (1 << 4)
#define PAGE_FAULT_PROTECTION_KEY (1 << 5)
#define PAGE_FAULT_SHADOW_STACK (1 << 6)
#define PAGE_FAULT_SOFTWARE_GUARD_EXTENSION (1 << 15)

extern "C"
{
	GDTPointer IDTLimits;
}

InterruptWithContext ISR_Callbacks[IDT_SIZE];
void* ISR_Contexts[IDT_SIZE];
uint16_t ISR_Selector;

bool GIsDebuggerPresent = false;

void __attribute__((used, noinline)) OnKernelMainHook()
{
    //Need some instructions to insert an interrupt into
    asm volatile("nop;nop;nop;nop;");
}

void __attribute__((used,noinline)) DebuggerHook()
{
    //Need some instructions to insert an interrupt into
    asm volatile("nop;nop;nop;nop;");
}

bool IsDebuggerPresent()
{
    return GIsDebuggerPresent;
}

#define STOP() 			\
	DebuggerHook(); 	\
	if (IsDebuggerPresent())\
	{DebugBreak();} 	\
	else{HaltPermanently();}

void AccessViolationCommon(uint64_t rip, uint64_t cr2, uint64_t errorCode)
{
	char16_t Buffer[32];

	if(errorCode & PAGE_FAULT_SHADOW_STACK)
	{
		ConsolePrint(u"Shadow stack violation ");
	}
	else if(cr2 == 0)
	{
		ConsolePrint(u"Null pointer exception ");
	}
	else
	{
		ConsolePrint(u"Access violation ");
	}

	if(errorCode & PAGE_FAULT_EXECUTE)
	{
		ConsolePrint(u"executing ");
	}
	else if(errorCode & PAGE_FAULT_WRITE)
	{
		ConsolePrint(u"writing ");
	}
	else
	{
		ConsolePrint(u"reading ");
	}

	if(errorCode & PAGE_FAULT_PRESENT)
	{
		ConsolePrint(u"unpaged ");
	}

	if(errorCode & PAGE_FAULT_USER)
	{
		ConsolePrint(u"user ");
	}
	else
	{
		ConsolePrint(u"kernel ");
	}

	if(errorCode & PAGE_FAULT_RESERVED_WRITE)
	{
		ConsolePrint(u"RESERVED WRITE ");
	}

	if(errorCode & PAGE_FAULT_PROTECTION_KEY)
	{
		ConsolePrint(u"PROTECTION KEY ");
	}

	if(errorCode & PAGE_FAULT_SOFTWARE_GUARD_EXTENSION)
	{
		ConsolePrint(u"SGX ");
	}

	ConsolePrint(u"(error 0x");
	witoabuf(Buffer, errorCode, 16);
	ConsolePrint(Buffer);
	ConsolePrint(u")\n");

	ConsolePrint(u" address 0x");
	witoabuf(Buffer, cr2, 16);
	ConsolePrint(Buffer);
	ConsolePrint(u"\n");

	ConsolePrint(u"\nRIP: 0x");
	witoabuf(Buffer, rip, 16);
	ConsolePrint(Buffer);

    PrintStackTrace(30);
}

DEFINE_NAMED_INTERRUPT(UnsetExceptionHandler)(uint64_t rip, uint64_t cr2, uint64_t errorCode, uint32_t interruptNumber)
{
	STOP();

	OutPort(0x20, 0x20);
}

void __attribute__((used,noinline)) AccessViolationException(uint64_t rip, uint64_t cr2, uint64_t errorCode)
{
	AccessViolationCommon(rip, cr2, errorCode);
	STOP();
}

DEFINE_NAMED_INTERRUPT(PageFault)(uint64_t rip, uint64_t cr2, uint64_t errorCode, uint32_t interruptNumber)
{
	//Paging will go here eventually

	AccessViolationException(rip, cr2, errorCode);

	OutPort(0x20, 0x20);
}

DEFINE_NAMED_INTERRUPT(Breakpoint)(uint64_t rip, uint64_t cr2, uint64_t errorCode, uint32_t interruptNumber)
{
	STOP();

	OutPort(0x20, 0x20);
}

//GCC bug! If this is not tagged with noinline, if this function can be inlined it
//this inlined function will be labelled as an ISR itself and it'll generate:
// "error : interrupt service routine cannot be called directly" which is incorrect.
void __attribute__((used,noinline)) InterruptDummy(const char16_t* message, int64_t rip, uint64_t cr2, uint64_t errorCode, uint32_t interruptNumber, bool terminate, bool hook)
{
	if(!hook)
	{
		char16_t Buffer[32];

		ConsolePrint(message);
		ConsolePrint(u"\nError code: 0x");
		witoabuf(Buffer, errorCode, 16);
		ConsolePrint(Buffer);
		ConsolePrint(u"\nRIP: 0x");
		witoabuf(Buffer, rip, 16);
		ConsolePrint(Buffer);
		ConsolePrint(u"\nCR2: 0x");
		witoabuf(Buffer, cr2, 16);
		ConsolePrint(Buffer);
		ConsolePrint(u"\n");
	}

    bool debuggerPresent = IsDebuggerPresent();

    if (hook)
    {
		if(interruptNumber != 3)
		{
			PrintStackTrace(30);
		}

        DebuggerHook();
    }

	if(terminate)
	{
		if (debuggerPresent)
		{
			DebugBreak();
		}
		else
		{
			HaltPermanently();
		}
	}
}

static uint32_t DefaultInterrupt(void* Context)
{
	STOP();
}

extern "C" void __attribute__((naked)) ISR_InterruptHandlerWithContext(void);

DEFINE_NAMED_INTERRUPT(InterruptHandlerWithContext)(uint64_t rip, uint64_t cr2, uint64_t errorCode, uint32_t interruptNumber)
{
	ISR_Callbacks[interruptNumber](ISR_Contexts[interruptNumber]);

	OutPort(0x20, 0x20);
}

void SetInterruptHandler(int InterruptNumber, InterruptWithContext Handler, void* Context)
{
	ISR_Callbacks[InterruptNumber] = Handler;
	ISR_Contexts[InterruptNumber] = Context;
}

void ClearInterruptHandler(int InterruptNumber)
{
	SetInterruptHandler(InterruptNumber, DefaultInterrupt, nullptr);
}

#define CALLBACK_INTERRUPT(n) \
	extern "C" void __attribute__((naked)) ISR_Callback##n(void); \
	extern "C" void KERNEL_API __attribute__((used,noinline)) ISR_Int_Callback##n(uint64_t rip, uint64_t cr2, uint64_t errorCode, uint32_t interruptNumber) \
	{ ISR_Int_InterruptHandlerWithContext(rip, cr2, errorCode, interruptNumber); }

#define NAMED_INTERRUPT(functionName) extern "C" void __attribute__((naked)) ISR_##functionName(void); extern "C" void KERNEL_API __attribute__((used,noinline)) ISR_Int_##functionName(uint64_t rip, uint64_t cr2, uint64_t errorCode, uint32_t interruptNumber);

#define PRINT_NAMED_INTERRUPT(functionName, message, hook) extern "C" void __attribute__((naked)) ISR_##functionName(void); extern "C" void KERNEL_API __attribute__((used,noinline)) ISR_Int_##functionName(uint64_t rip, uint64_t cr2, uint64_t errorCode, uint32_t interruptNumber){ InterruptDummy(message, rip, cr2, errorCode, interruptNumber, false, hook); }
#define PRINT_NAMED_INTERRUPT_HALT(functionName, message) extern "C" void __attribute__((naked)) ISR_##functionName(void); extern "C" void KERNEL_API __attribute__((used,noinline)) ISR_Int_##functionName(uint64_t rip, uint64_t cr2, uint64_t errorCode, uint32_t interruptNumber){ InterruptDummy(message, rip, cr2, errorCode, interruptNumber, true, true); }

PRINT_NAMED_INTERRUPT(DivideByZero, u"Divide by 0", true) //0
PRINT_NAMED_INTERRUPT_HALT(SingleStep, u"Single step") //1
PRINT_NAMED_INTERRUPT_HALT(NonMaskableInterrupt, u"Non-maskable Interrupt") //2
NAMED_INTERRUPT(Breakpoint) //3
PRINT_NAMED_INTERRUPT_HALT(Overflow, u"Overflow") //4
PRINT_NAMED_INTERRUPT_HALT(BoundRangeExceeded, u"Bound range exceeded") //5
PRINT_NAMED_INTERRUPT_HALT(InvalidOpcode, u"Invalid opcode") //6
PRINT_NAMED_INTERRUPT(CoprocessorUnavailable, u"Coprocessor unavailable", true) //7
PRINT_NAMED_INTERRUPT_HALT(DoubleFault, u"Double fault") //8
CALLBACK_INTERRUPT(9) // Coprocessor Segment Overrun is not used 9
PRINT_NAMED_INTERRUPT(InvalidTaskStateSegment, u"Invalid task state segment", true) //10
PRINT_NAMED_INTERRUPT_HALT(SegmentNotPresent, u"Segment not present") //11
PRINT_NAMED_INTERRUPT_HALT(StackSegmentFault, u"Stack segment fault") //12
PRINT_NAMED_INTERRUPT_HALT(GeneralProtectionFault, u"General protection fault") //13 0xD
NAMED_INTERRUPT(PageFault) //14 0xE
CALLBACK_INTERRUPT(15) //15 is reserved
PRINT_NAMED_INTERRUPT(x87FloatingPointException, u"x87 Floating point exception", true) //16
PRINT_NAMED_INTERRUPT(AlignmentCheck, u"Alignment check", true) //17
PRINT_NAMED_INTERRUPT(MachineCheck, u"Machine check", false) //18
PRINT_NAMED_INTERRUPT(SIMDFloatingPointException, u"SIMD floating point exception", true) //19
PRINT_NAMED_INTERRUPT(VirtualizationException, u"Virtualization exception", true) //20 0x14
PRINT_NAMED_INTERRUPT(ControlProtectionException, u"Control protection exception", true) //21
CALLBACK_INTERRUPT(22)
CALLBACK_INTERRUPT(23)
CALLBACK_INTERRUPT(24)
CALLBACK_INTERRUPT(25)
CALLBACK_INTERRUPT(26)
CALLBACK_INTERRUPT(27)
CALLBACK_INTERRUPT(28)
CALLBACK_INTERRUPT(29)
CALLBACK_INTERRUPT(30)
CALLBACK_INTERRUPT(31)//22-31 are reserved
NAMED_INTERRUPT(PITInterrupt) //32
CALLBACK_INTERRUPT(33)
CALLBACK_INTERRUPT(34)
CALLBACK_INTERRUPT(35)
CALLBACK_INTERRUPT(36)
CALLBACK_INTERRUPT(37)
CALLBACK_INTERRUPT(38)
CALLBACK_INTERRUPT(39)
CALLBACK_INTERRUPT(40)
CALLBACK_INTERRUPT(41)
CALLBACK_INTERRUPT(42)
CALLBACK_INTERRUPT(43)
CALLBACK_INTERRUPT(44)
CALLBACK_INTERRUPT(45)
CALLBACK_INTERRUPT(46)
CALLBACK_INTERRUPT(47)
CALLBACK_INTERRUPT(48)
CALLBACK_INTERRUPT(49)
CALLBACK_INTERRUPT(50)
CALLBACK_INTERRUPT(51)
CALLBACK_INTERRUPT(52)
CALLBACK_INTERRUPT(53)
CALLBACK_INTERRUPT(54)
CALLBACK_INTERRUPT(55)
CALLBACK_INTERRUPT(56)
CALLBACK_INTERRUPT(57)
CALLBACK_INTERRUPT(58)
CALLBACK_INTERRUPT(59)
CALLBACK_INTERRUPT(60)
CALLBACK_INTERRUPT(61)
CALLBACK_INTERRUPT(62)
CALLBACK_INTERRUPT(63)
CALLBACK_INTERRUPT(64)
CALLBACK_INTERRUPT(65)
CALLBACK_INTERRUPT(66)
CALLBACK_INTERRUPT(67)
CALLBACK_INTERRUPT(68)
CALLBACK_INTERRUPT(69)

NAMED_INTERRUPT(SpuriousInterrupt) //0xFF

#define SET_INTERRUPT(num) \
{ \
uint64_t ISRAddress = (uint64_t)&ISR_Callback##num; \
IDT[num].OffsetLow = (uint16_t)(ISRAddress & 0xFFFF); \
IDT[num].OffsetMid = (uint16_t)((ISRAddress >> 16) & 0xFFFF); \
IDT[num].OffsetHigh = (uint32_t)((ISRAddress >> 32) & 0xFFFFFFFF); \
IDT[num].Selector = codeSelector * sizeof(GDTEntry); \
IDT[num].IST = 0; \
IDT[num].Flags = TrapGate; \
IDT[num].Reserved = 0; \
}

#define SET_NAMED_INTERRUPT(num, functionName) \
{ \
uint64_t ISRAddress = (uint64_t)&ISR_##functionName; \
IDT[num].OffsetLow = (uint16_t)(ISRAddress & 0xFFFF); \
IDT[num].OffsetMid = (uint16_t)((ISRAddress >> 16) & 0xFFFF); \
IDT[num].OffsetHigh = (uint32_t)((ISRAddress >> 32) & 0xFFFFFFFF); \
IDT[num].Selector = codeSelector * sizeof(GDTEntry); \
IDT[num].IST = 0; \
IDT[num].Flags = InterruptGate; \
IDT[num].Reserved = 0; \
}

#define SET_NAMED_TRAP(num, functionName) \
{ \
uint64_t ISRAddress = (uint64_t)&ISR_##functionName; \
IDT[num].OffsetLow = (uint16_t)(ISRAddress & 0xFFFF); \
IDT[num].OffsetMid = (uint16_t)((ISRAddress >> 16) & 0xFFFF); \
IDT[num].OffsetHigh = (uint32_t)((ISRAddress >> 32) & 0xFFFFFFFF); \
IDT[num].Selector = codeSelector * sizeof(GDTEntry); \
IDT[num].IST = 0; \
IDT[num].Flags = TrapGate; \
IDT[num].Reserved = 0; \
}

static_assert(sizeof(IDTEntry) == 0x10);

void DisableInterrupts()
{
    __asm("cli");
}

void EnableInterrupts()
{
    __asm("sti");
}

DEFINE_NAMED_INTERRUPT(SpuriousInterrupt)(uint64_t rip, uint64_t cr2, uint64_t errorCode, uint32_t interruptNumber)
{
}

void InitializeDefaultInterrupts(uint8_t* IDTPtr, unsigned int codeSelector)
{
	ISR_Selector = codeSelector * sizeof(GDTEntry);

	IDTEntry* IDT = (IDTEntry*)IDTPtr;

    SET_NAMED_TRAP(0, DivideByZero) //0
    SET_NAMED_TRAP(1, SingleStep) //1
    SET_NAMED_TRAP(2, NonMaskableInterrupt) //2
    SET_NAMED_TRAP(3, Breakpoint) //3
    SET_NAMED_TRAP(4, Overflow) //4
    SET_NAMED_TRAP(5, BoundRangeExceeded) //5
    SET_NAMED_TRAP(6, InvalidOpcode) //6
    SET_NAMED_TRAP(7, CoprocessorUnavailable) //7
    SET_NAMED_TRAP(8, DoubleFault) //8
    SET_INTERRUPT(9) // Coprocessor Segment Overrun is not used
    SET_NAMED_TRAP(10, InvalidTaskStateSegment) //10
    SET_NAMED_TRAP(11, SegmentNotPresent) //11
    SET_NAMED_TRAP(12, StackSegmentFault) //12
    SET_NAMED_INTERRUPT(13, GeneralProtectionFault) //13 0xD
    SET_NAMED_TRAP(14, PageFault) //14 0xE
    SET_INTERRUPT(15) // 15 is reserved 0xF
    SET_NAMED_TRAP(16, x87FloatingPointException) //16
    SET_NAMED_TRAP(17, AlignmentCheck) //17
    SET_NAMED_TRAP(18, MachineCheck) //18
    SET_NAMED_TRAP(19, SIMDFloatingPointException) //19
    SET_NAMED_TRAP(20, VirtualizationException) //20 0x14
    SET_NAMED_TRAP(21, ControlProtectionException) //21
    SET_INTERRUPT(22) //22-31 are reserved
	SET_INTERRUPT(23)
	SET_INTERRUPT(24)
	SET_INTERRUPT(25)
	SET_INTERRUPT(26)
	SET_INTERRUPT(27)
	SET_INTERRUPT(28)
	SET_INTERRUPT(29)
	SET_INTERRUPT(30)
	SET_INTERRUPT(31)
    SET_NAMED_INTERRUPT(32, PITInterrupt) //32
    SET_INTERRUPT(33)
    SET_INTERRUPT(34)
    SET_INTERRUPT(35)
    SET_INTERRUPT(36)
    SET_INTERRUPT(37)
    SET_INTERRUPT(38)
    SET_INTERRUPT(39)
    SET_INTERRUPT(40)
    SET_INTERRUPT(41)
    SET_INTERRUPT(42)
    SET_INTERRUPT(43)
    SET_INTERRUPT(44)
    SET_INTERRUPT(45)
    SET_INTERRUPT(46)
    SET_INTERRUPT(47)
    SET_INTERRUPT(48)
    SET_INTERRUPT(49)
    SET_INTERRUPT(50)
    SET_INTERRUPT(51)
    SET_INTERRUPT(52)
    SET_INTERRUPT(53)
    SET_INTERRUPT(54)
    SET_INTERRUPT(55)
    SET_INTERRUPT(56)
    SET_INTERRUPT(57)
    SET_INTERRUPT(58)
    SET_INTERRUPT(59)
    SET_INTERRUPT(60)
    SET_INTERRUPT(61)
    SET_INTERRUPT(62)
    SET_INTERRUPT(63)
    SET_INTERRUPT(64)
    SET_INTERRUPT(65)
    SET_INTERRUPT(66)
    SET_INTERRUPT(67)
    SET_INTERRUPT(68)
    SET_INTERRUPT(69)

	SET_NAMED_INTERRUPT(0xFF, SpuriousInterrupt)
}

size_t InitIDT(uint8_t* TargetMemory, unsigned int codeSelector)
{
	size_t IDTSize = sizeof(IDTEntry) * IDT_SIZE;

    IDTLimits.Limit = IDTSize - 1;
    IDTLimits.Base = (uint64_t)TargetMemory;

    memset(TargetMemory, 0, IDTSize);
    InitializeDefaultInterrupts(TargetMemory, codeSelector);

    asm volatile("lidt %0" : : "m"(IDTLimits));

	return IDTSize;
}
