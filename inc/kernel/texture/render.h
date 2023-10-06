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
