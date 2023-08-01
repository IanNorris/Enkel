#include <stdint.h>
#include <stddef.h>

#include "Jura.h"
#include "Jura_0.h"
#include "common/string.h"
#include "font/details/bmfont.h"
#include "memory/memory.h"
#include "utilities/termination.h"

#include "kernel/bootload.h"
#include "kernel/gdt.h"

BMFont GDefaultFont;
KernelBootData GBootData;

extern "C" void AssertionFailed(const char* expression, const char* message, const char* filename, size_t lineNumber)
{
    char16_t messageBuffer[2048];

    BMFontColor Background = { 0x54, 0x00, 0x2C };
    uint32_t BackgroundU32 = Background.Blue | (Background.Green << 8) | (Background.Red << 16);

    memset32(GBootData.Framebuffer, BackgroundU32, 240 * GBootData.FramebufferPitch);

    int OriginX = 20;

    int StartX = OriginX;
    int StartY = 20;

    BMFontColor Header = { 0xFF, 0x7B, 0x0 }; //Red/Orange
    BMFontColor Label = { 0x8c, 0xFF, 0x0 }; //Light Green
    BMFontColor Text = { 0x00, 0x7F, 0xFF }; //Mid Blue

    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, u"ASSERTION FAILED", Header);
    StartY += 2* GDefaultFont.Common->LineHeight;

    StartX = OriginX;
    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, u"Message: ", Label);
    ascii_to_wide(messageBuffer, message, 2048);
    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, messageBuffer, Text);
    StartY += GDefaultFont.Common->LineHeight;

    StartX = OriginX;
    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, u"Expression: ", Label);
    ascii_to_wide(messageBuffer, expression, 2048);
    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, messageBuffer, Text);
    StartY += GDefaultFont.Common->LineHeight;

    StartX = OriginX;
    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, u"Filename: ", Label);
    ascii_to_wide(messageBuffer, filename, 2048);
    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, messageBuffer, Text);
    StartY += GDefaultFont.Common->LineHeight;

    StartX = OriginX;
    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, u"Line: ", Label);
    witoabuf(messageBuffer, lineNumber, 10);
    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, messageBuffer, Text);
    StartY += GDefaultFont.Common->LineHeight;

    halt();
    while (1);
}

void Slow();

void InterruptDummy(const char16_t* Str, struct interrupt_frame* frame, bool terminate)
{
    (void)frame;

    BMFontColor Col = { 0x8c, 0xFF, 0x0 };
    static int StartX = 200;
    static int StartY = 300;
    int CurrentX = StartX;
    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, Str, Col);
    StartX = CurrentX;
    StartY += GDefaultFont.Common->LineHeight;
    if (StartY >= 1000)
    {
        StartX += 200;
        StartY = 300;
    }

    Slow();

    if (terminate)
    {
        halt();
    }
}

extern "C" void SetGDT(uint16_t limit, uint64_t base);
extern "C" void ReloadSegments();

#define PRINT_INTERRUPT(n) extern "C" void __attribute__((interrupt)) Interrupt_##n(struct interrupt_frame* frame) \
{ (void)frame; InterruptDummy(u"Interrupt " #n, frame, false);}

#define PRINT_NAMED_INTERRUPT(functionName, string) extern "C" void __attribute__((interrupt)) functionName(struct interrupt_frame* frame) {InterruptDummy(string, frame, false);}
#define PRINT_NAMED_INTERRUPT_HALT(functionName, string) extern "C" void __attribute__((interrupt)) functionName(struct interrupt_frame* frame) {InterruptDummy(string, frame, true);}

PRINT_NAMED_INTERRUPT(Interrupt_Div0, u"Divide by 0") //0
PRINT_NAMED_INTERRUPT_HALT(Interrupt_SingleStep, u"Single step") //1
PRINT_NAMED_INTERRUPT_HALT(Interrupt_NonMaskableInterrupt, u"Non-maskable Interrupt") //2
PRINT_NAMED_INTERRUPT(Interrupt_Breakpoint, u"Breakpoint") //3
PRINT_NAMED_INTERRUPT_HALT(Interrupt_Overflow, u"Overflow") //4
PRINT_NAMED_INTERRUPT_HALT(Interrupt_BoundRangeExceeded, u"Bound range exceeded") //5
PRINT_NAMED_INTERRUPT_HALT(Interrupt_InvalidOpcode, u"Invalid opcode") //6
PRINT_NAMED_INTERRUPT(Interrupt_CoprocessorUnavailable, u"Coprocessor unavailable") //7
PRINT_NAMED_INTERRUPT_HALT(Interrupt_DoubleFault, u"Double fault") //8
PRINT_NAMED_INTERRUPT(Interrupt_CoprocessorSegmentOverrun, u"Coprocessor segment overrun") //9
PRINT_NAMED_INTERRUPT(Interrupt_InvalidTaskStateSegment, u"Invalid task state segment") //10
PRINT_NAMED_INTERRUPT_HALT(Interrupt_SegmentNotPresent, u"Segment not present") //11
PRINT_NAMED_INTERRUPT_HALT(Interrupt_StackSegmentFault, u"Stack segment fault") //12
PRINT_NAMED_INTERRUPT_HALT(Interrupt_GeneralProtectionFault, u"General protection fault") //13
PRINT_NAMED_INTERRUPT(Interrupt_PageFault, u"Page fault") //14
//15 is reserved
PRINT_NAMED_INTERRUPT(Interrupt_x87FloatingPointException, u"x87 Floating point exception") //16
PRINT_NAMED_INTERRUPT(Interrupt_AlignmentCheck, u"Alignment check") //17
PRINT_NAMED_INTERRUPT(Interrupt_MachineCheck, u"Machine check") //18
PRINT_NAMED_INTERRUPT(Interrupt_SIMDFloatingPointException, u"SIMD floating point exception") //19
PRINT_NAMED_INTERRUPT(Interrupt_VirtualizationException, u"Virtualization exception") //20
//21-29 are reserved

PRINT_INTERRUPT(30)
PRINT_INTERRUPT(31)
PRINT_INTERRUPT(32)
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

#define KERNEL_CODE_SEGMENT 1

#define SET_INTERRUPT(num) \
{ \
uint64_t ISRAddress = (uint64_t)&Interrupt_##num; \
IDT[num].OffsetLow = (uint16_t)(ISRAddress & 0xFFFF); \
IDT[num].OffsetMid = (uint16_t)((ISRAddress >> 16) & 0xFFFF); \
IDT[num].OffsetHigh = (uint32_t)((ISRAddress >> 32) & 0xFFFFFFFF); \
IDT[num].Selector = KERNEL_CODE_SEGMENT * sizeof(GDTEntry); \
IDT[num].IST = 0; \
IDT[num].Flags = TrapGate; \
IDT[num].Reserved = 0; \
}

#define SET_NAMED_INTERRUPT(num, functionName) \
{ \
uint64_t ISRAddress = (uint64_t)&functionName; \
IDT[num].OffsetLow = (uint16_t)(ISRAddress & 0xFFFF); \
IDT[num].OffsetMid = (uint16_t)((ISRAddress >> 16) & 0xFFFF); \
IDT[num].OffsetHigh = (uint32_t)((ISRAddress >> 32) & 0xFFFFFFFF); \
IDT[num].Selector = KERNEL_CODE_SEGMENT * sizeof(GDTEntry); \
IDT[num].IST = 0; \
IDT[num].Flags = InterruptGate; \
IDT[num].Reserved = 0; \
}

#define SET_NAMED_TRAP(num, functionName) \
{ \
uint64_t ISRAddress = (uint64_t)&functionName; \
IDT[num].OffsetLow = (uint16_t)(ISRAddress & 0xFFFF); \
IDT[num].OffsetMid = (uint16_t)((ISRAddress >> 16) & 0xFFFF); \
IDT[num].OffsetHigh = (uint32_t)((ISRAddress >> 32) & 0xFFFFFFFF); \
IDT[num].Selector = KERNEL_CODE_SEGMENT * sizeof(GDTEntry); \
IDT[num].IST = 0; \
IDT[num].Flags = TrapGate; \
IDT[num].Reserved = 0; \
}

#define IDT_SIZE 256
static IDTEntry IDT[IDT_SIZE];
static GDTPointer IDTLimits;

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
static_assert(sizeof(IDTEntry) == 0x10);

void Slow()
{
    for (int i = 0; i < 10000000; i++)
    {
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
        asm volatile("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;");
    }
}

void InitializeLongMode()
{
    //Disable interrupts because we won't be able to handle these until we initialize the GDT and the interrupt handlers.
    __asm("cli");

    memset(IDT, 0, sizeof(IDT));

    SET_NAMED_TRAP(0, Interrupt_Div0) //0
    SET_NAMED_TRAP(1, Interrupt_SingleStep) //1
    SET_NAMED_TRAP(2, Interrupt_NonMaskableInterrupt) //2
    SET_NAMED_TRAP(3, Interrupt_Breakpoint) //3
    SET_NAMED_TRAP(4, Interrupt_Overflow) //4
    SET_NAMED_TRAP(5, Interrupt_BoundRangeExceeded) //5
    SET_NAMED_TRAP(6, Interrupt_InvalidOpcode) //6
    SET_NAMED_TRAP(7, Interrupt_CoprocessorUnavailable) //7
    SET_NAMED_TRAP(8, Interrupt_DoubleFault) //8
    SET_NAMED_TRAP(9, Interrupt_CoprocessorSegmentOverrun) //9
    SET_NAMED_TRAP(10, Interrupt_InvalidTaskStateSegment) //10
    SET_NAMED_TRAP(11, Interrupt_SegmentNotPresent) //11
    SET_NAMED_TRAP(12, Interrupt_StackSegmentFault) //12
    SET_NAMED_INTERRUPT(13, Interrupt_GeneralProtectionFault) //13
    //SET_NAMED_TRAP(13, ISRWrapper) //13
    SET_NAMED_TRAP(14, Interrupt_PageFault) //14
    // 15 is reserved
    SET_NAMED_TRAP(16, Interrupt_x87FloatingPointException) //16
    SET_NAMED_TRAP(17, Interrupt_AlignmentCheck) //17
    SET_NAMED_TRAP(18, Interrupt_MachineCheck) //18
    SET_NAMED_TRAP(19, Interrupt_SIMDFloatingPointException) //19
    SET_NAMED_TRAP(20, Interrupt_VirtualizationException) //20
    // 21-29 are reserved
    
    SET_INTERRUPT(30)
    SET_INTERRUPT(31)
    SET_INTERRUPT(32)
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

    IDTLimits.Limit = sizeof(IDT) - 1;
    IDTLimits.Base = (uint64_t)IDT;

    GDTLimits.Limit = sizeof(GDT) - 1;
    GDTLimits.Base = (uint64_t)GDT;

    int StartX = 0;
    int StartY = 40;
    BMFontColor Col = { 0x8c, 0xFF, 0x0 }; //Light Green

    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, u"1\n", Col);
    Slow();

    //Load the GDT
    SetGDT((sizeof(GDTEntry) * 3) - 1, (uint64_t)&GDT[0]);

    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, u"2\n", Col);
    Slow();

    //Force a jump to apply all the changes
    ReloadSegments();

    //Load the IDT
    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, u"3\n", Col);
    Slow();

    asm volatile("lidt %0" : : "m"(IDTLimits));

    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, u"4\n", Col);
    Slow();
        
    //We can now re-enable interrupts.
    __asm("sti");

    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, u"5\n", Col);
    Slow();
}

void Div0()
{
    int StartX = 400;
    int StartY = 0;
    BMFontColor Col = { 0x8c, 0xFF, 0x0 }; //Light Green

    int x = 1000;
    int y = 100;
    int z = 0;
    while (y > -2)
    {
        z+= x / (--y);
    }

    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, u"Done\n" + (z > 0 ? 1 : 0), Col);
    Slow();
}

extern "C" void __attribute__((sysv_abi, __noreturn__)) /*__attribute__((__noreturn__))*/ KernelMain(KernelBootData* bootData)
{
    GBootData = *bootData;

    BMFont_Load(Jura_fnt_data, Jura_fnt_size, &GDefaultFont);
    GDefaultFont.PageTextureData[0] = Jura_0_tga_data;
    GDefaultFont.PageTextureSize[0] = Jura_0_tga_size;


    //AssertionFailed("1==2", "This is a test", __FILE__, __LINE__);

    ///bootData->GraphicsOutput->SetMode(bootData->GraphicsOutput, 4);

    memset(bootData->Framebuffer, 0x66, 200 * bootData->FramebufferPitch);

    int StartX = 100;
    int StartY = 0;
    BMFontColor Col = { 0x8c, 0xFF, 0x0 }; //Light Green
    BMFont_Render(&GDefaultFont, bootData->Framebuffer, bootData->FramebufferPitch, StartX, StartY, u"Starting Enkel...\n", Col);
    Slow();

    InitializeLongMode();

    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, u"6\n", Col);
    Slow();

    asm volatile("int $3");

    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, u"7\n", Col);
    Slow();

    Div0();

    BMFont_Render(&GDefaultFont, GBootData.Framebuffer, GBootData.FramebufferPitch, StartX, StartY, u"Exiting\n", Col);
    Slow();

    halt();
    while (1);
}
