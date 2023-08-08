#pragma once

enum SlowType
{
	Flush,
	Blip,
	Dawdle,
	Loitering,
	LoooooooongCat,
	LoooooooongLoooooooongMan,
};

void Slow(SlowType type = Loitering);
