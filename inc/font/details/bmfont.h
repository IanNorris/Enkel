#pragma once

#include "kernel/framebuffer/framebuffer.h"

#define BMFont_MaxPages 8

#pragma pack(push, 1)

struct BMFontHeader;
struct BMFontInfo;
struct BMFontCommon;
struct BMFontPage;
struct BMFontChar;
struct BMFontKerningPair;

struct BMFontColor
{
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
};

#pragma pack(pop)

struct BMFont
{
    uint32_t TotalCharacters;
    uint32_t TotalKerningPairs;

    BMFontHeader* Header;
    BMFontInfo* Info;
    BMFontCommon* Common;
    BMFontPage* Pages;
    BMFontChar* Chars;
    BMFontKerningPair* KerningPairs;

    const unsigned char* PageTextureData[BMFont_MaxPages];
    unsigned int PageTextureSize[BMFont_MaxPages];
};

bool BMFont_Load(const void* FileData, uint32_t FileSize, BMFont* FontOut);
void BMFont_Render(const BMFont* Font, FramebufferLayout* Framebuffer, int32_t& StartX, int32_t& StartY, int32_t ReturnX, const char16_t* String, const BMFontColor& Color);
