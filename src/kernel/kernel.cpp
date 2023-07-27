#include <stdint.h>
#include <stddef.h>

#include "Jura.h"
#include "Jura_0.h"
#include "common/string.h"
#include "font/details/bmfont.h"
#include "memory/memory.h"
#include "utilities/termination.h"

#include "kernel/bootload.h"

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
}

extern "C" void __attribute__((sysv_abi, __noreturn__)) /*__attribute__((__noreturn__))*/ KernelMain(KernelBootData* bootData)
{
    BMFont Font;
    GBootData = *bootData;

    BMFont_Load(Jura_fnt_data, Jura_fnt_size, &Font);
    Font.PageTextureData[0] = Jura_0_tga_data;
    Font.PageTextureSize[0] = Jura_0_tga_size;
    BMFont_Load(Jura_fnt_data, Jura_fnt_size, &GDefaultFont);
    GDefaultFont.PageTextureData[0] = Jura_0_tga_data;
    GDefaultFont.PageTextureSize[0] = Jura_0_tga_size;


    AssertionFailed("1==2", "This is a test", __FILE__, __LINE__);

    ///bootData->GraphicsOutput->SetMode(bootData->GraphicsOutput, 4);

    memset(bootData->Framebuffer, 0x66, 200 * bootData->FramebufferPitch);

    BMFont_Render(&Font, bootData->Framebuffer, bootData->FramebufferPitch, 0, 0, L"Hello World!\n\u01C4\u00C9AM\nThis is a test.... @~`2584\n#\uEFEF");
    int StartX = 0;
    int StartY = 0;
    BMFontColor Col = { 0x8c, 0xFF, 0x0 }; //Light Green
    BMFont_Render(&GDefaultFont, bootData->Framebuffer, bootData->FramebufferPitch, StartX, StartY, u"Starting Enkel...", Col);
    
    //We're dead Dave.
    halt();
    while(1);
}
