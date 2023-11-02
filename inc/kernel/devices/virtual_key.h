#pragma once

// Great reference for scan codes across layouts:
// https://kbdlayout.info/kbdukx/virtualkeys

enum class ScanCode
{
	Null,
	
	Escape 			= 0x1,
	Num1,
	Num2,
	Num3,
	Num4,
	Num5,
	Num6,
	Num7,
	Num8,
	Num9,
	Num0,

	LayoutSpecific2,
	LayoutSpecific3,
	Backspace,
	Tab,

	Q,
	W,
	E,
	R,
	T,
	Y,
	U,
	I,
	O,
	P,

	LayoutSpecific4 = 0x1A,
	LayoutSpecific5,
	
	Enter,

	ControlLeft,

	A				= 0x1E,
	S,
	D,
	F,
	G,
	H,
	J,
	K,
	L,

	LayoutSpecific6	= 0x27,
	LayoutSpecific7,
	LayoutSpecific1,

	ShiftLeft,

	LayoutSpecific8,

	Z				= 0x2C,
	X,
	C,
	V,
	B,
	N,
	M,

	Comma,
	Period,

	LayoutSpecific9,

	ShiftRight,
	Multiply,

	Alt,
	Space,
	CapsLock,

	Function1		= 0x3B,
	Function2,
	Function3,
	Function4,
	Function5,
	Function6,
	Function7,
	Function8,
	Function9,
	Function10,
	
	NumLock			= 0x45,
	SrollLock,

	NumPad7,
	NumPad8,
	NumPad9,
	NumPadSubtract,
	NumPad4,
	NumPad5,
	NumPad6,
	NumPaddAdd,
	NumPad1,
	NumPad2,
	NumPad3,
	NumPad0,
	NumPadDecimal,

	PrintScreen		= 0x54,

	LayoutSpecific10= 0x56,

	Function11,
	Function12,

	RangeEnd_Standard,

	/////////////////////////////////////////////////////////////////////////////////////////

	RangeStart_Extended1 = 0xE010, //Must match lowest E0 value
	MediaTrackPrevious	= 0xE010,
	MediaTrackNext	= 0xE019,

	NumPadEnter		= 0xE01C,
	ControlRight	= 0xE01D,

	MediaMute		= 0xE020,
	MediaPlay		= 0xE022,
	MediaStop		= 0xE024,
	MediaVolumeDown = 0xE02E,
	MediaVolumeUp	= 0xE030,

	NumPadDivide	= 0xE035, 
	AltGr	 		= 0xE038,

	Home			= 0xE047,
	Up				= 0xE048,
	PageUp			= 0xE049,
	Left			= 0xE04B,
	Right			= 0xE04D,
	End				= 0xE04F,
	Down			= 0xE050,
	PageDown		= 0xE051,
	Insert			= 0xE052,
	Delete			= 0xE053,
	WinLeft			= 0xE05B,
	WinRight		= 0xE05C,
	Emoji			= 0xE05D,
	RangeEnd_Extended1 = 0xE06E,

	/////////////////////////////////////////////////////////////////////////////////////////

	RangeStart_Extended2 = 0xE11D,
	Pause			= 0xE11D,
	RangeEnd_Extended2,

	/////////////////////////////////////////////////////////////////////////////////////////

	RangeLength_Standard = RangeEnd_Standard,
	RangeLength_Extended1 = RangeEnd_Extended1 - RangeStart_Extended1,
	RangeLength_Extended2 = RangeEnd_Extended2 - RangeStart_Extended2,

	MAX
};
