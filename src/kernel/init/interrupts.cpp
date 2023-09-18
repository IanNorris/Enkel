#include <stdint.h>
#include <stddef.h>

#include "memory/memory.h"
#include "utilities/termination.h"

#include "kernel/console/console.h"
#include "kernel/init/gdt.h"
#include "kernel/init/msr.h"
#include "common/string.h"

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

    if (debuggerPresent)
    {

    }
    else
    {
        if (terminate)
        {
            HaltPermanently();
        }
    }
}

#define PRINT_INTERRUPT(n) extern "C" void __attribute__((naked)) ISR_Unused##n(void); extern "C" void KERNEL_API __attribute__((used,noinline)) ISR_Int_Unused##n(uint64_t rip, uint64_t cr2, uint64_t errorCode, uint32_t interruptNumber){ InterruptDummy(u"Interrupt " #n, rip, cr2, errorCode, interruptNumber, false, false); }

#define NAMED_INTERRUPT(functionName) extern "C" void __attribute__((naked)) ISR_##functionName(void); extern "C" void KERNEL_API __attribute__((used,noinline)) ISR_Int_##functionName(uint64_t rip, uint64_t cr2, uint64_t errorCode, uint32_t interruptNumber);

#define PRINT_NAMED_INTERRUPT(functionName, message, hook) extern "C" void __attribute__((naked)) ISR_##functionName(void); extern "C" void KERNEL_API __attribute__((used,noinline)) ISR_Int_##functionName(uint64_t rip, uint64_t cr2, uint64_t errorCode, uint32_t interruptNumber){ InterruptDummy(message, rip, cr2, errorCode, interruptNumber, false, hook); }
#define PRINT_NAMED_INTERRUPT_HALT(functionName, message) extern "C" void __attribute__((naked)) ISR_##functionName(void); extern "C" void KERNEL_API __attribute__((used,noinline)) ISR_Int_##functionName(uint64_t rip, uint64_t cr2, uint64_t errorCode, uint32_t interruptNumber){ InterruptDummy(message, rip, cr2, errorCode, interruptNumber, true, true); }

PRINT_NAMED_INTERRUPT(DivideByZero, u"Divide by 0", true) //0
PRINT_NAMED_INTERRUPT_HALT(SingleStep, u"Single step") //1
PRINT_NAMED_INTERRUPT_HALT(NonMaskableInterrupt, u"Non-maskable Interrupt") //2
PRINT_NAMED_INTERRUPT(Breakpoint, u"Breakpoint", true) //3
PRINT_NAMED_INTERRUPT_HALT(Overflow, u"Overflow") //4
PRINT_NAMED_INTERRUPT_HALT(BoundRangeExceeded, u"Bound range exceeded") //5
PRINT_NAMED_INTERRUPT_HALT(InvalidOpcode, u"Invalid opcode") //6
PRINT_NAMED_INTERRUPT(CoprocessorUnavailable, u"Coprocessor unavailable", true) //7
PRINT_NAMED_INTERRUPT_HALT(DoubleFault, u"Double fault") //8
// Coprocessor Segment Overrun is not used
PRINT_NAMED_INTERRUPT(InvalidTaskStateSegment, u"Invalid task state segment", true) //10
PRINT_NAMED_INTERRUPT_HALT(SegmentNotPresent, u"Segment not present") //11
PRINT_NAMED_INTERRUPT_HALT(StackSegmentFault, u"Stack segment fault") //12
PRINT_NAMED_INTERRUPT_HALT(GeneralProtectionFault, u"General protection fault") //13
PRINT_NAMED_INTERRUPT(PageFault, u"Page fault", false) //14
//15 is reserved
PRINT_NAMED_INTERRUPT(x87FloatingPointException, u"x87 Floating point exception", true) //16
PRINT_NAMED_INTERRUPT(AlignmentCheck, u"Alignment check", true) //17
PRINT_NAMED_INTERRUPT(MachineCheck, u"Machine check", false) //18
PRINT_NAMED_INTERRUPT(SIMDFloatingPointException, u"SIMD floating point exception", true) //19
PRINT_NAMED_INTERRUPT(VirtualizationException, u"Virtualization exception", true) //20
PRINT_NAMED_INTERRUPT(ControlProtectionException, u"Control protection exception", true) //21
//22-31 are reserved
NAMED_INTERRUPT(PITInterrupt) //32
PRINT_INTERRUPT(33)
PRINT_INTERRUPT(34)
PRINT_INTERRUPT(35)
PRINT_INTERRUPT(36)
PRINT_INTERRUPT(37)
PRINT_INTERRUPT(38)
PRINT_INTERRUPT(39)
PRINT_INTERRUPT(40)
PRINT_INTERRUPT(41)
PRINT_INTERRUPT(42)
PRINT_INTERRUPT(43)
PRINT_INTERRUPT(44)
PRINT_INTERRUPT(45)
PRINT_INTERRUPT(46)
PRINT_INTERRUPT(47)
PRINT_INTERRUPT(48)
PRINT_INTERRUPT(49)
PRINT_INTERRUPT(50)
PRINT_INTERRUPT(51)
PRINT_INTERRUPT(52)
PRINT_INTERRUPT(53)
PRINT_INTERRUPT(54)
PRINT_INTERRUPT(55)
PRINT_INTERRUPT(56)
PRINT_INTERRUPT(57)
PRINT_INTERRUPT(58)
PRINT_INTERRUPT(59)
PRINT_INTERRUPT(60)
PRINT_INTERRUPT(61)
PRINT_INTERRUPT(62)
PRINT_INTERRUPT(63)
PRINT_INTERRUPT(64)
PRINT_INTERRUPT(65)
PRINT_INTERRUPT(66)
PRINT_INTERRUPT(67)
PRINT_INTERRUPT(68)
PRINT_INTERRUPT(69)

#define SET_INTERRUPT(num) \
{ \
uint64_t ISRAddress = (uint64_t)&ISR_Unused##num; \
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

#define IDT_SIZE 256

static_assert(sizeof(IDTEntry) == 0x10);

void DisableInterrupts()
{
    __asm("cli");
}

void EnableInterrupts()
{
    __asm("sti");
}

void InitializeDefaultInterrupts(uint8_t* IDTPtr, unsigned int codeSelector)
{
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
    // Coprocessor Segment Overrun is not used
    SET_NAMED_TRAP(10, InvalidTaskStateSegment) //10
    SET_NAMED_TRAP(11, SegmentNotPresent) //11
    SET_NAMED_TRAP(12, StackSegmentFault) //12
    SET_NAMED_INTERRUPT(13, GeneralProtectionFault) //13
    //SET_NAMED_TRAP(13, ISRWrapper) //13
    SET_NAMED_TRAP(14, PageFault) //14
    // 15 is reserved
    SET_NAMED_TRAP(16, x87FloatingPointException) //16
    SET_NAMED_TRAP(17, AlignmentCheck) //17
    SET_NAMED_TRAP(18, MachineCheck) //18
    SET_NAMED_TRAP(19, SIMDFloatingPointException) //19
    SET_NAMED_TRAP(20, VirtualizationException) //20
    SET_NAMED_TRAP(21, ControlProtectionException) //21
    //22-31 are reserved
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
}

size_t InitIDT(uint8_t* TargetMemory, unsigned int codeSelector)
{
	GDTPointer IDTLimits;

	size_t IDTSize = sizeof(IDTEntry) * IDT_SIZE;

    IDTLimits.Limit = IDTSize - 1;
    IDTLimits.Base = (uint64_t)TargetMemory;

    memset(TargetMemory, 0, IDTSize);
    InitializeDefaultInterrupts(TargetMemory, codeSelector);

    asm volatile("lidt %0" : : "m"(IDTLimits));

	return IDTSize;
}
