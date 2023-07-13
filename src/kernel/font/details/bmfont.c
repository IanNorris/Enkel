#include "font/details/bmfont.h"
#include "memory/memory.h"

#define INC_SIZE(size)		\
	FileDataPtr += size;	\
	FileSize -= size;

bool BMFont_Load(const void* FileData, uint32_t FileSize, BMFont* FontOut)
{
	memset(FontOut, 0, sizeof(BMFont));

	const uint8_t* FileDataPtr = (const uint8_t*)FileData;

	FontOut->Header = (BMFontHeader*)FileDataPtr;
	
	INC_SIZE(sizeof(BMFontHeader));
	
	BMFontBlock* NextBlock = (BMFontBlock*)FileDataPtr;
	while (FileSize > sizeof(BMFontBlock))
	{
		INC_SIZE(sizeof(BMFontBlock));

		switch (NextBlock->Id)
		{
		case BMFontBlockType_Info:
			FontOut->Info = (BMFontInfo*)FileDataPtr;
			break;

		case BMFontBlockType_Common:
			FontOut->Common = (BMFontCommon*)FileDataPtr;
			break;

		case BMFontBlockType_Pages:
			//Pages = (BMFontPages*)FileDataPtr;
			break;

		case BMFontBlockType_Chars:
			FontOut->TotalCharacters = NextBlock->Size / sizeof(BMFontChar);
			FontOut->Chars = (BMFontChar*)FileDataPtr;
			break;

		case BMFontBlockType_KerningPairs:
			FontOut->TotalKerningPairs = NextBlock->Size / sizeof(BMFontKerningPair);
			FontOut->KerningPairs = (BMFontKerningPair*)FileDataPtr;
			break;
		}
		INC_SIZE(NextBlock->Size);
		NextBlock = (BMFontBlock*)FileDataPtr;
	}

	//assert(FileSize == 0);

	return true;
}

BMFontChar* BMFont_FindGlyph(BMFont* Font, uint32_t Id)
{
	uint32_t Start = 0;
	uint32_t End = Font->TotalCharacters - 1;

	while (Start <= End)
	{
		uint32_t Mid = Start + (End - Start) / 2;

		if (Font->Chars[Mid].Id == Id)
		{
			return &Font->Chars[Mid];
		}

		if (Font->Chars[Mid].Id < Id)
		{
			Start = Mid + 1;
		}
		else
		{
			if (Mid == 0)
			{
				return nullptr;
			}
			End = Mid - 1;
		}
	}

	return nullptr;
}

void BMFont_RenderGlyph(BMFont* Font, BMFontChar* Glyph, void* Framebuffer, uint32_t FramebufferPitch, int32_t StartX, int32_t StartY)
{
	uint32_t* PageTextureBytes = (uint32_t*)(Font->PageTextureData[Glyph->Page]+18);
	uint32_t* FramebufferU32 = (uint32_t*)Framebuffer;
	uint32_t PagePitch = Font->Common->ScaleW;
	uint32_t FramebufferQuadPitch = FramebufferPitch / 4;

	uint32_t Mask = 0xFFFFFF00;
	uint32_t Shift = 0;

	// (1 = blue, 2 = green, 4 = red, 8 = alpha, 15 = all channels).
	switch(Glyph->Channel)
	{
	case 1: // Blue
		{
			Mask = 0x000000FF;
			Shift = 0;
			break;
		}

	case 2: // Green
		{
			Mask = 0x000FF00;
			Shift = 8;
			break;
		}

	case 4: // Red
		{
			Mask = 0x00FF0000;
			Shift = 16;
			break;
		}

	case 8: // Alpha
		{
			Mask = 0xFF000000;
			Shift = 24;
			break;
		}
	}
		
	for (int32_t GlyphX = 0; GlyphX < Glyph->Width; GlyphX++)
	{
		if (GlyphX + StartX < 0)
		{
			continue;
		}

		for (int32_t GlyphY = 0; GlyphY < Glyph->Height; GlyphY++)
		{
			if (GlyphY + StartY < 0)
			{
				continue;
			}

			uint32_t ReadPos = ((GlyphY + Glyph->Y) * PagePitch) + GlyphX + Glyph->X;
			uint32_t GlyphChannel = (PageTextureBytes[ReadPos] & Mask) >> Shift;
			
			if (GlyphChannel)
			{
				uint32_t GlyphColor = GlyphChannel | (GlyphChannel << 8) | (GlyphChannel << 16) | (GlyphChannel << 24);

				uint32_t OutputPos = ((GlyphY + StartY) * FramebufferQuadPitch) + GlyphX + StartX;
				FramebufferU32[OutputPos] = GlyphColor;
			}
		}
	}
}

void BMFont_Render(BMFont* Font, void* Framebuffer, uint32_t FramebufferPitch, int32_t StartX, int32_t StartY, const wchar_t* String)
{
	int32_t OriginalStartX = StartX;
	while (*String)
	{
		if (*String == '\n')
		{
			StartX = OriginalStartX;
			StartY += Font->Common->LineHeight;
			String++;
			continue;
		}

		BMFontChar* Glyph = BMFont_FindGlyph(Font, *String);

		if (!Glyph)
		{
			Glyph = BMFont_FindGlyph(Font, 0xFFFD);
		}

		if (!Glyph)
		{
			return;
		}

		BMFont_RenderGlyph(Font, Glyph, Framebuffer, FramebufferPitch, StartX + Glyph->XOffset, StartY + Glyph->YOffset);
		StartX += Glyph->XAdvance;

		String++;
	}

}

#undef INC_SIZE
