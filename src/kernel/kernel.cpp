#include <stdint.h>
#include <stddef.h>

#include "Jura.h"
#include "Jura_0.h"
#include "font/details/bmfont.h"
#include "memory/memory.h"
#include "utilities/termination.h"

#include "kernel/bootload.h"

extern "C" void AssertionFailed(const char* expression, const char* message, const char* filename, size_t lineNumber)
{
    halt();
}

extern "C" void __attribute__((sysv_abi, __noreturn__)) /*__attribute__((__noreturn__))*/ KernelMain(KernelBootData* bootData)
{
    BMFont Font;

    BMFont_Load(Jura_fnt_data, Jura_fnt_size, &Font);
    Font.PageTextureData[0] = Jura_0_tga_data;
    Font.PageTextureSize[0] = Jura_0_tga_size;

    ///bootData->GraphicsOutput->SetMode(bootData->GraphicsOutput, 4);

    memset(bootData->Framebuffer, 0x66, 200 * bootData->FramebufferPitch);

    BMFont_Render(&Font, bootData->Framebuffer, bootData->FramebufferPitch, 0, 0, L"Hello World!\n\u01C4\u00C9AM\nThis is a test.... @~`2584\n#\uEFEF");
    
    //We're dead Dave.
    halt();
    while(1);
}
