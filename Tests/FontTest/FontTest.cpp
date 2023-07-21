#include <iostream>

#include "font/details/bmfont.h"
#include "../../src/kernel/font/details/bmfont.cpp"

//Include the font data directly
#include "../../fonts/Jura.h"
#include "../../fonts/Jura.cpp"

#include "../../fonts/Jura_0.h"
#include "../../fonts/Jura_0.cpp"

int main()
{
    BMFont Font;

    const int FrameBufferPitch = 800 * 4;
    void* FrameBuffer = malloc(1024 * 1024 * 4);
    memset(FrameBuffer, 0x33, 1024 * 1024 * 4);

    BMFont_Load(Jura_fnt_data, Jura_fnt_size, &Font);
    Font.PageTextureData[0] = Jura_0_tga_data;
    Font.PageTextureSize[0] = Jura_0_tga_size;

    //BMFont_Render(&Font, FrameBuffer, FrameBufferPitch, 0, 0, "Hello World!");
    BMFont_Render(&Font, FrameBuffer, FrameBufferPitch, 0, 0, L"Hello World! \u03B2\n\u01C4\u00C9AM\nThis is a test.... @~`2584\n#\uEFEF\n Sîne klâwen durh die wolken sint geslagen,\nНа берегу пустынных волн\n");
}
