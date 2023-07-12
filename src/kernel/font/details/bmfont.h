#pragma once

#pragma pack(push, 1)

struct BMFontHeader
{
    char FileIdentifier[3];
    uint8_t FormatVersion;
};

struct BMFontBlock
{
    uint8_t BlockType;
    int32_t BlockSize;
};

struct BlockTypeInfo
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
};

struct BlockTypeCommon
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
};

struct BlockTypePages
{
    char** PageNames; // Actual size determined by BlockSize
};

struct BlockTypeChars
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
};

struct BlockTypeKerningPairs
{
    uint32_t First;
    uint32_t Second;
    int16_t Amount;
};

#pragma pack(pop)
