#pragma once

#include "kernel/init/bootload.h"

enum class AlignImage
{
	Top = 0,
	Left = 0,

	Middle = 1,

	Right = 2,
	Bottom = 2,
};

void RenderTGA(FramebufferLayout* Framebuffer, const unsigned char* imageBuffer, int32_t StartX, int32_t StartY, AlignImage AlignX, AlignImage AlignY, bool transparent);
bool RenderQR(FramebufferLayout* Framebuffer, const char* Data, uint32_t StartX, uint32_t StartY, uint32_t PixelsPerModule, bool BlackModules, int contrast = 3 /*0-16*/);
uint32_t ApplyTransparency(uint32_t color, uint8_t alpha, uint32_t background);
