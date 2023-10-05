#include "kernel/console/console.h"
#include "utilities/termination.h"

#include "memory/physical.h"

#include "common/string.h"
#include "font/details/bmfont_internal.h"
#include "font/details/bmfont.h"

void Halt(void)
{
    asm("hlt");
}

void DebugBreak()
{
	asm volatile("int $3");
}

void __attribute__((used, noinline, noreturn)) HaltPermanently(void)
{
    //Stop interrupts
    asm("cli");
    while(true)
    {
        asm("hlt");
    }
}

void __attribute__((used, noinline)) PrintStackTrace(int maxFrames)
{
	char16_t buffer[2048];

	//Skip ourself
	StackFrame* Top = GetCurrentStackFrame();
	StackFrame* Prev = nullptr;
	int stackIndex = 0;
	while(true)
	{
		if(		Top == nullptr 
			|| 	Top == Prev
			||	Top->RIP == 0
			||	maxFrames == 0)
		{
			break;
		}

		bool InvalidPointer = 
				(uint64_t)GetPhysicalAddress((uint64_t)Top) == INVALID_ADDRESS
			||	(uint64_t)GetPhysicalAddress((uint64_t)Top + sizeof(StackFrame)) == INVALID_ADDRESS;

		if(InvalidPointer)
		{
			ConsolePrint(u"#0x");
			witoabuf(buffer, stackIndex, 16);
			ConsolePrint(buffer);

			ConsolePrint(u": 0x");
			witoabuf(buffer, Top->RIP, 16);
			ConsolePrint(buffer);

			ConsolePrint(u" INVALID ADDRESS!\n");
			break;
		}

		ConsolePrint(u"#0x");
		witoabuf(buffer, stackIndex, 16);
		ConsolePrint(buffer);

		ConsolePrint(u": 0x");
		witoabuf(buffer, Top->RIP, 16);
		ConsolePrint(buffer);

		ConsolePrint(u"\n");
		
		Prev = Top;
		maxFrames--;
		stackIndex++;

		Top = Top->RBP;
	}
}

void KERNEL_NORETURN AssertionFailed(const char* expression, const char* message, const char* filename, size_t lineNumber, uint64_t v1, uint64_t v2, uint64_t v3)
{
    char16_t messageBuffer[2048];

    //BMFontColor Background = { 0x54, 0x00, 0x2C };
    //uint32_t BackgroundU32 = Background.Blue | (Background.Green << 8) | (Background.Red << 16);
    //memset32(GBootData.Framebuffer, BackgroundU32, 240 * GBootData.FramebufferPitch);

    const BMFont* Font = GetFont();

    int OriginX = 0;

    int StartX = 0;
    int StartY = -1;

    BMFontColor Header = { 0xFF, 0x7B, 0x0 }; //Red/Orange
    BMFontColor Label = { 0x8c, 0xFF, 0x0 }; //Light Green
    BMFontColor Text = { 0xFF, 0xFF, 0x00 }; //Yellow

	StartY = -1;
    ConsolePrintAtPosWithColor(u"\n\nASSERTION FAILED", StartX, StartY, OriginX, Header);
    StartY += Font->Common->LineHeight;

    StartX = OriginX;
    ConsolePrintAtPosWithColor(u"Message: ", StartX, StartY, OriginX, Label);
    ascii_to_wide(messageBuffer, message, 2048);
    ConsolePrintAtPosWithColor(messageBuffer, StartX, StartY, OriginX, Text);
    StartY += Font->Common->LineHeight;

    StartX = OriginX;
    ConsolePrintAtPosWithColor(u"Expression: ", StartX, StartY, OriginX, Label);
    ascii_to_wide(messageBuffer, expression, 2048);
    ConsolePrintAtPosWithColor(messageBuffer, StartX, StartY, OriginX, Text);
    StartY += Font->Common->LineHeight;

    StartX = OriginX;
    ConsolePrintAtPosWithColor(u"Filename: ", StartX, StartY, OriginX, Label);
    ascii_to_wide(messageBuffer, filename, 2048);
    ConsolePrintAtPosWithColor(messageBuffer, StartX, StartY, OriginX, Text);
    StartY += Font->Common->LineHeight;

    StartX = OriginX;
    ConsolePrintAtPosWithColor(u"Line: ", StartX, StartY, OriginX, Label);
    witoabuf(messageBuffer, lineNumber, 10);
    ConsolePrintAtPosWithColor(messageBuffer, StartX, StartY, OriginX, Text);
    StartY += Font->Common->LineHeight;

	StartX = OriginX;
    ConsolePrintAtPosWithColor(u"Value1: 0x", StartX, StartY, OriginX, Label);
    witoabuf(messageBuffer, v1, 16);
    ConsolePrintAtPosWithColor(messageBuffer, StartX, StartY, OriginX, Text);
    StartY += Font->Common->LineHeight;

	StartX = OriginX;
    ConsolePrintAtPosWithColor(u"Value2: 0x", StartX, StartY, OriginX, Label);
    witoabuf(messageBuffer, v2, 16);
    ConsolePrintAtPosWithColor(messageBuffer, StartX, StartY, OriginX, Text);
    StartY += Font->Common->LineHeight;

	StartX = OriginX;
    ConsolePrintAtPosWithColor(u"Value3: 0x", StartX, StartY, OriginX, Label);
    witoabuf(messageBuffer, v3, 16);
    ConsolePrintAtPosWithColor(messageBuffer, StartX, StartY, OriginX, Text);
    StartY += Font->Common->LineHeight;

	ConsolePrint(u"\n\n\n\n\n\n\n\n\n\nStack trace:\n");

	PrintStackTrace(60);

	DebugBreak();
    HaltPermanently();
}
