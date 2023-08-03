#pragma once

#include "common/types.h"

struct FramebufferLayout
{
	uint32_t* Base;
	uint32_t Width;
	uint32_t Height;
	uint32_t Pitch;
};
