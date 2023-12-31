#include "font/details/bmfont_internal.h"
#include "font/details/bmfont.h"
#include "memory/memory.h"
#include "common/string.h"

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

const BMFontChar* BMFont_FindGlyph(const BMFont* Font, uint32_t Id)
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

void BMFont_RenderGlyph(const BMFont* Font, const BMFontChar* Glyph, FramebufferLayout* Framebuffer, int32_t StartX, int32_t StartY, const BMFontColor& Color)
{
	uint32_t* PageTextureBytes = (uint32_t*)(Font->PageTextureData[Glyph->Page]+18);
	uint32_t* FramebufferU32 = (uint32_t*)Framebuffer->Base;
	uint32_t PagePitch = Font->Common->ScaleW;
	uint32_t FramebufferQuadPitch = Framebuffer->Pitch / 4;

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

		if (GlyphX + StartX >= (int32_t)Framebuffer->Width)
		{
			continue;
		}

		for (int32_t GlyphY = 0; GlyphY < Glyph->Height; GlyphY++)
		{
			if (GlyphY + StartY < 0)
			{
				continue;
			}

			if (GlyphY + StartY >= (int32_t)Framebuffer->Height)
			{
				continue;
			}

			uint32_t ReadPos = ((GlyphY + Glyph->Y) * PagePitch) + GlyphX + Glyph->X;
			uint32_t GlyphChannel = (PageTextureBytes[ReadPos] & Mask) >> Shift;
			
			if (GlyphChannel)
			{
				uint32_t RedChannel = (Color.Red * GlyphChannel) / 255;
				uint32_t GreenChannel = (Color.Green * GlyphChannel) / 255;
				uint32_t BlueChannel = (Color.Blue * GlyphChannel) / 255;
				
				uint32_t GlyphColor = BlueChannel | (GreenChannel << 8) | (RedChannel << 16);

				uint32_t OutputPos = ((GlyphY + StartY) * FramebufferQuadPitch) + GlyphX + StartX;
				FramebufferU32[OutputPos] = GlyphColor;
			}
		}
	}
}

void BMFont_Render(const BMFont* Font, FramebufferLayout* Framebuffer, int32_t& StartX, int32_t& StartY, int32_t ReturnX, const char16_t* String, const BMFontColor& Color)
{
	int Index = 0;
	int NextOffset = -1;
	int BreakAfterChar = -1;
	while (*String)
	{
		if (*String == '\r')
		{
			String++;
			Index++;
			continue;
		}

		//We're attempting to find the next whitespace to see if we need to break at the end of the line
		if (NextOffset == -1)
		{
			NextOffset = IndexOfWhitespace(String, 0);
			if (NextOffset == -1)
			{
				NextOffset = strlen(String);
			}

			//Calculate the length of the next word
			uint32_t NextWordPixelLength = 0;
			for (int AdvanceChar = 0; AdvanceChar < NextOffset; AdvanceChar++)
			{
				const BMFontChar* Glyph = BMFont_FindGlyph(Font, *(String + AdvanceChar));
				if(!Glyph)
				{
					continue;
				}

				if (NextWordPixelLength + Glyph->XAdvance > Framebuffer->Width)
				{
					BreakAfterChar = AdvanceChar;
					goto escapeLongString;
				}

				NextWordPixelLength += Glyph->XAdvance;
			}

			//If we're longer than the line
			if (StartX + NextWordPixelLength > Framebuffer->Width)
			{
				//If we're longer than screen width
				if (NextWordPixelLength > Framebuffer->Width)
				{
					//Do nothing here as BreakAfterChar will be triggered above anyway.
				}
				else
				{
					StartX = ReturnX;
					StartY += Font->Common->LineHeight;
					continue;
				}
			}
		}

escapeLongString:

		if (BreakAfterChar != -1 && --BreakAfterChar == 0)
		{
			BreakAfterChar = -1;
			StartX = ReturnX;
			StartY += Font->Common->LineHeight;
			NextOffset = -1;
			continue;
		}
		else if (*String == '\n')
		{
			BreakAfterChar = -1;
			StartX = ReturnX;
			StartY += Font->Common->LineHeight;
			String++;
			if (*String == ' ')
			{
				String++;
			}
			continue;
		}

		const BMFontChar* Glyph = BMFont_FindGlyph(Font, *String);

		if (!Glyph)
		{
			Glyph = BMFont_FindGlyph(Font, 0xFFFD);
		}

		if (!Glyph)
		{
			return;
		}

		BMFont_RenderGlyph(Font, Glyph, Framebuffer, StartX + Glyph->XOffset, StartY + Glyph->YOffset, Color);
		StartX += Glyph->XAdvance;

		String++;
		Index++;

		if (NextOffset != -1)
		{
			NextOffset--;
		}
	}

}

#undef INC_SIZE
