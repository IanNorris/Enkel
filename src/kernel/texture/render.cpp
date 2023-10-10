#include "kernel/init/bootload.h"
#include "kernel/texture/render.h"
#include "kernel/texture/tga.h"

#include "qrcode.h"

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
			
			if (TextureChannel)
			{
				uint32_t OutputPos = (((height-y) + StartY) * FramebufferQuadPitch) + x + StartX;
				FramebufferU32[OutputPos] = TextureChannel;
			}
		}
	}
}

void RenderQR(FramebufferLayout* Framebuffer, const QRCode* QR, uint32_t StartX, uint32_t StartY, uint32_t PixelsPerModule, bool BlackModules)
{
	uint32_t* FramebufferU32 = (uint32_t*)Framebuffer->Base;
	uint32_t FramebufferQuadPitch = Framebuffer->Pitch / 4;

	uint32_t ModuleColour = BlackModules ? 0x0 : 0xFFFFFFFF;

	for (uint32_t y = 0; y < QR->size; y++)
	{
		for (uint32_t x = 0; x < QR->size; x++)
		{
			uint32_t posX = x * PixelsPerModule;
			uint32_t posY = y * PixelsPerModule;
			
			bool Set = qrcode_getModule((QRCode*)QR, x, y);

			if(Set)
			{
				for (uint32_t moduleY = 0; moduleY < PixelsPerModule; moduleY++)
				{
					for (uint32_t moduleX = 0; moduleX < PixelsPerModule; moduleX++)
					{
						uint32_t OutputPos = ((posY + moduleY + StartY) * FramebufferQuadPitch) + posX + moduleX + StartX;
						FramebufferU32[OutputPos] = ModuleColour;
					}
				}
			}
		}
	}
}