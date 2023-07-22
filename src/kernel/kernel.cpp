#include <stdint.h>
#include <stddef.h>
//#include <limine.h>

#include "Jura.h"
#include "Jura_0.h"
#include "font/details/bmfont.h"
#include "memory/memory.h"
#include "utilities/termination.h"

#include "kernel/bootload.h"

/*
// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void) {
    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        halt();
    }

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
	
    BMFont Font;

    BMFont_Load(Jura_fnt_data, Jura_fnt_size, &Font);
    Font.PageTextureData[0] = Jura_0_tga_data;
    Font.PageTextureSize[0] = Jura_0_tga_size;

	memset(framebuffer->address, 0x66, 200 * framebuffer->pitch);

	BMFont_Render(&Font, framebuffer->address, framebuffer->pitch, 0, 0, L"Hello World!\n\u01C4\u00C9AM\nThis is a test.... @~`2584\n#\uEFEF");

    // We're done, just hang...
    halt();
}
*/

extern "C" void AssertionFailed(const char* expression, const char* message, const char* filename, size_t lineNumber)
{
    halt();
}

extern "C" void __attribute__((sysv_abi)) /*__attribute__((__noreturn__))*/ KernelMain(KernelBootData* bootData)
{
    bootData->Sum = 3 + 4;

    bootData->Print(u"Hello from the Kernel!\r\n");

    /*BMFont Font;

    BMFont_Load(Jura_fnt_data, Jura_fnt_size, &Font);
    Font.PageTextureData[0] = Jura_0_tga_data;
    Font.PageTextureSize[0] = Jura_0_tga_size;

    memset(bootData.Framebuffer, 0x66, 200 * bootData.FramebufferPitch);

    BMFont_Render(&Font, bootData.Framebuffer, bootData.FramebufferPitch, 0, 0, L"Hello World!\n\u01C4\u00C9AM\nThis is a test.... @~`2584\n#\uEFEF");
    */
    //We're dead Dave.
    //halt();
    //while(1);
}
