#include "kernel/init/bootload.h"
#include "kernel/texture/render.h"
#include "kernel/texture/tga.h"

#include "qrcodegen.h"

// Helper function to blend two 8-bit color components
uint8_t BlendColourComponent(uint8_t src, uint8_t dst, uint8_t alpha)
{
	return (src * alpha + dst * (255 - alpha)) / 255;
}

// Function to apply transparency to a 24-bit color value
uint32_t ApplyTransparency(uint32_t color, uint8_t alpha, uint32_t background)
{
	uint8_t r = BlendColourComponent((color >> 16) & 0xFF, (background >> 16) & 0xFF, alpha);
	uint8_t g = BlendColourComponent((color >> 8) & 0xFF, (background >> 8) & 0xFF, alpha);
	uint8_t b = BlendColourComponent(color & 0xFF, background & 0xFF, alpha);
	return (r << 16) | (g << 8) | b;
}

void RenderTGA(FramebufferLayout* Framebuffer, const unsigned char* imageBuffer, int32_t StartX, int32_t StartY, AlignImage AlignX, AlignImage AlignY, bool transparent)
{
	const TGAHeader& header = *(const TGAHeader*)imageBuffer;

	const int width = header.width;
	const int height = header.height;

	switch(AlignX)
	{
		case AlignImage::Left:
			break;

		case AlignImage::Middle:
			StartX -= width / 2;
			break;

		case AlignImage::Right:
			StartX -= width;
			break;
	}

	switch(AlignY)
	{
		case AlignImage::Top:
			break;

		case AlignImage::Middle:
			StartY -= height / 2;
			break;

		case AlignImage::Bottom:
			StartY -= height;
			break;
	}

	const uint32_t* Texture = (uint32_t*)(imageBuffer+18);
	uint32_t* FramebufferU32 = (uint32_t*)Framebuffer->Base;
	uint32_t PagePitch = width;
	uint32_t FramebufferQuadPitch = Framebuffer->Pitch / 4;

	uint32_t transparentMask = transparent ? 0x00FFFFFF : 0xFFFFFFFF;

	for (int32_t y = height-1; y >= 0; y--)
	{
		if (y + StartY < 0)
		{
			continue;
		}

		if (y + StartY >= (int32_t)Framebuffer->Height)
		{
			continue;
		}
		
		for (int32_t x = 0; x < width; x++)
		{
			if (x + StartX < 0)
			{
				continue;
			}

			if (x + StartX >= (int32_t)Framebuffer->Width)
			{
				continue;
			}

			uint32_t ReadPos = (y * PagePitch) + x;
			uint32_t TextureChannel = Texture[ReadPos] & transparentMask;
			uint32_t Transparency = (Texture[ReadPos] & ~transparentMask) >> 24;
			
			if (TextureChannel)
			{
				uint32_t OutputPos = (((height-y) + StartY) * FramebufferQuadPitch) + x + StartX;
				if (Transparency)
				{
					FramebufferU32[OutputPos] = ApplyTransparency(TextureChannel, Transparency, FramebufferU32[OutputPos]);
				}
				else
				{
					FramebufferU32[OutputPos] = TextureChannel;
				}
			}
		}
	}
}

bool RenderQR(FramebufferLayout* Framebuffer, const char* Data, uint32_t StartX, uint32_t StartY, uint32_t PixelsPerModule, bool BlackModules, int contrast)
{
	uint8_t qr0[qrcodegen_BUFFER_LEN_MAX];
	uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];

	if (!qrcodegen_isAlphanumeric(Data))
	{
		_ASSERTF(false, "Input data for QR code is not base45");
		return false;
	}

	bool ok = qrcodegen_encodeText(Data, tempBuffer, qr0, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
	if (!ok)
	{
		_ASSERTF(false, "Failed to generate QR code");
		return false;
	}

	int size = qrcodegen_getSize(qr0);
	
	uint32_t* FramebufferU32 = (uint32_t*)Framebuffer->Base;
	uint32_t FramebufferQuadPitch = Framebuffer->Pitch / 4;

	const uint32_t Black = 0x00000000;
	const uint32_t White = 0xFFFFFFFF - (contrast * 0x11111111);

	uint32_t ModuleColour = BlackModules ? Black : White;
	uint32_t BackgroundColour = BlackModules ? White : Black;

	int borderwidth = 2;

	for (int32_t y = -borderwidth; y < size + borderwidth; y++)
	{
		for (int32_t x = -borderwidth; x < size + borderwidth; x++)
		{
			int32_t posX = (x * PixelsPerModule) + borderwidth;
			int32_t posY = (y * PixelsPerModule) + borderwidth;
			
			bool Valid = x >= 0 && x < size && y >= 0 && y < size;
			bool Set = Valid &&qrcodegen_getModule(qr0, x, y);

			for (uint32_t moduleY = 0; moduleY < PixelsPerModule; moduleY++)
			{
				for (uint32_t moduleX = 0; moduleX < PixelsPerModule; moduleX++)
				{
					uint32_t OutputPos = ((posY + moduleY + StartY) * FramebufferQuadPitch) + posX + moduleX + StartX;
					FramebufferU32[OutputPos] = Set ? ModuleColour : BackgroundColour;
				}
			}
		}
	}

	return true;
}