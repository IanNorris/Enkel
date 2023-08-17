#include "kernel/console/console.h"

#include "common/string.h"
#include "font/details/bmfont_internal.h"
#include "font/details/bmfont.h"

void Halt(void)
{
    asm("hlt");
}

void KERNEL_NORETURN HaltPermanently(void)
{
    //Stop interrupts
    asm("cli");
    while(true)
    {
        asm("hlt");
    }
}

void KERNEL_NORETURN AssertionFailed(const char* expression, const char* message, const char* filename, size_t lineNumber)
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

    HaltPermanently();
}
