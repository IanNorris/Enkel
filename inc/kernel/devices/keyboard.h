#pragma once

#include "kernel/devices/virtual_key.h"

typedef const char16_t* KeyString;
typedef const char16_t KeyChar;

struct KeyLayout
{
	KeyString Standard[(uint32_t)ScanCode::RangeLength_Standard];

	KeyString Standard_Shift[(uint32_t)ScanCode::RangeLength_Standard];
	KeyString Standard_AltGr[(uint32_t)ScanCode::RangeLength_Standard];
	KeyString Standard_ShiftAltGr[(uint32_t)ScanCode::RangeLength_Standard];
};

struct KeyState
{
	const static uint32_t ReleasedKeyOffset = 0x80;

	uint8_t Standard[(uint32_t)ScanCode::RangeLength_Standard];
	uint8_t Extended1[(uint32_t)ScanCode::RangeLength_Extended1];
	uint8_t Extended2[(uint32_t)ScanCode::RangeLength_Extended2];

	uint8_t ExtendedKeyState;

	inline uint8_t IsReleased(uint32_t key) const
	{
		if(key < (uint32_t)ScanCode::RangeEnd_Standard)
		{
			return false;
		}
		else if(key < (uint32_t)ScanCode::RangeEnd_Standard+ReleasedKeyOffset)
		{
			return true;
		}
		else if(key < (uint32_t)ScanCode::RangeEnd_Extended1)
		{
			return false;
		}
		else if(key < (uint32_t)ScanCode::RangeEnd_Extended1 + ReleasedKeyOffset)
		{
			return true;
		}
		else if(key < (uint32_t)ScanCode::RangeEnd_Extended2)
		{
			return false;
		}
		else if(key < (uint32_t)ScanCode::RangeEnd_Extended2 + ReleasedKeyOffset)
		{
			return true;
		}
		else
		{
			_ASSERTFV(false, "Invalid scan code", key, 0, 0);
		}
	}

	inline uint8_t GetKeyState(uint32_t key) const
	{
		if(key < (uint32_t)ScanCode::RangeEnd_Standard)
		{
			return Standard[key];
		}
		else if(key < (uint32_t)ScanCode::RangeEnd_Extended1)
		{
			return Extended1[key - (uint32_t)ScanCode::RangeStart_Extended1];
		}
		else if(key < (uint32_t)ScanCode::RangeEnd_Extended2)
		{
			return Extended2[key - (uint32_t)ScanCode::RangeStart_Extended2];
		}
		else
		{
			_ASSERTFV(false, "Invalid scan code", key, 0, 0);
		}
	}

	inline void UpdateKeyState(uint32_t key)
	{
		if(key < (uint32_t)ScanCode::RangeEnd_Standard)
		{
			Standard[key] = 1;
		}
		else if(key < (uint32_t)ScanCode::RangeEnd_Standard+ReleasedKeyOffset)
		{
			Standard[key-ReleasedKeyOffset] = 0;
		}
		else if(key < (uint32_t)ScanCode::RangeEnd_Extended1)
		{
			Extended1[key - (uint32_t)ScanCode::RangeStart_Extended1] = 1;
		}
		else if(key < (uint32_t)ScanCode::RangeEnd_Extended1 + ReleasedKeyOffset)
		{
			Extended1[key - (uint32_t)ScanCode::RangeStart_Extended1 - ReleasedKeyOffset] = 0;
		}
		else if(key < (uint32_t)ScanCode::RangeEnd_Extended2)
		{
			Extended1[key - (uint32_t)ScanCode::RangeStart_Extended2] = 1;
		}
		else if(key < (uint32_t)ScanCode::RangeEnd_Extended2 + ReleasedKeyOffset)
		{
			Extended1[key - (uint32_t)ScanCode::RangeStart_Extended2 - ReleasedKeyOffset] = 0;
		}
		else
		{
			_ASSERTFV(false, "Invalid scan code", key, 0, 0);
		}
	}

	inline KeyString GetPrintable(uint32_t key, const KeyLayout* layout) const
	{
		if(key < (uint32_t)ScanCode::RangeEnd_Standard)
		{
			bool shift = Standard[(uint32_t)ScanCode::ShiftLeft] || Standard[(uint32_t)ScanCode::ShiftRight];
			bool alt = Standard[(uint32_t)ScanCode::Alt];
			bool altGr = Extended1[(uint32_t)ScanCode::AltGr - (uint32_t)ScanCode::RangeStart_Extended1];
			bool control = Standard[(uint32_t)ScanCode::ControlLeft] || Standard[(uint32_t)ScanCode::ControlRight];

			if(control || alt)
			{
				return nullptr;
			}
			else if(shift && altGr)
			{
				return layout->Standard_ShiftAltGr[key];
			}
			else if(shift)
			{
				return layout->Standard_Shift[key];
			}
			else if(altGr)
			{
				return layout->Standard_AltGr[key];
			}
			else
			{
				return layout->Standard[key];
			}

			return nullptr;
		}
		
		return nullptr;
	}
};

void InitKeyboard();
