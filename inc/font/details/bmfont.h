#pragma once

#ifndef TESTBED
    #include "common/types.h"
#endif

#pragma pack(push, 1)

#define BMFont_MaxPages 8

enum BMFontBlockType
{
	BMFontBlockType_Info = 1,
	BMFontBlockType_Common = 2,
	BMFontBlockType_Pages = 3,
	BMFontBlockType_Chars = 4,
	BMFontBlockType_KerningPairs = 5
};

typedef struct
{
    char FileIdentifier[3];
    uint8_t FormatVersion;
}BMFontHeader;

typedef struct
{
    uint8_t Id;
    int32_t Size;
}BMFontBlock;

typedef struct
{
    int16_t FontSize;
    uint8_t Smooth : 1;
    uint8_t Unicode : 1;
    uint8_t Italic : 1;
    uint8_t Bold : 1;
    uint8_t FixedHeight : 1;
    uint8_t Reserved : 3;
    uint8_t CharSet;
    uint16_t StretchH;
    uint8_t AA;
    uint8_t PaddingUp;
    uint8_t PaddingRight;
    uint8_t PaddingDown;
    uint8_t PaddingLeft;
    uint8_t SpacingHoriz;
    uint8_t SpacingVert;
    uint8_t Outline;
    char FontName[1]; // Actual size determined by BlockSize
}BMFontInfo;

typedef struct
{
    uint16_t LineHeight;
    uint16_t Base;
    uint16_t ScaleW;
    uint16_t ScaleH;
    uint16_t Pages;
    uint8_t Reserved : 7;
    uint8_t Packed : 1;
    uint8_t AlphaChnl;
    uint8_t RedChnl;
    uint8_t GreenChnl;
    uint8_t BlueChnl;
}BMFontCommon;

typedef struct
{
    char PageNames[1]; // Actual size determined by BlockSize
}BMFontPage;

typedef struct
{
    uint32_t Id;
    uint16_t X;
    uint16_t Y;
    uint16_t Width;
    uint16_t Height;
    int16_t XOffset;
    int16_t YOffset;
    int16_t XAdvance;
    uint8_t Page;
    uint8_t Channel;
}BMFontChar;

typedef struct
{
    uint32_t First;
    uint32_t Second;
    int16_t Amount;
}BMFontKerningPair;

typedef struct
{
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
}BMFontColor;

#pragma pack(pop)

typedef struct
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
}BMFont;

bool BMFont_Load(const void* FileData, uint32_t FileSize, BMFont* FontOut);
void BMFont_Render(BMFont* Font, void* Framebuffer, uint32_t FramebufferPitch, int32_t& StartX, int32_t& StartY, const char16_t* String, const BMFontColor& Color);
