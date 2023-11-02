#include "kernel/console/console.h"
#include "kernel/devices/keyboard.h"
#include "kernel/init/interrupts.h"
#include "kernel/init/msr.h"
#include "kernel/init/pic.h"
#include "common/string.h"
#include "memory/memory.h"

KeyLayout* CurrentKeyboardLayout;

KeyLayout KeyboardLayout_British = 
{
	//Standard
	{
		nullptr,

		nullptr, //Escape
		u"1",
		u"2",
		u"3",
		u"4",
		u"5",
		u"6",
		u"7",
		u"8",
		u"9",
		u"0",

		u"-", //LayoutSpecific2
		u"=", //LayoutSpecific3
		nullptr, //Backspace
		u"\t", //Tab

		u"q",
		u"w",
		u"e",
		u"r",
		u"t",
		u"y",
		u"u",
		u"i",
		u"o",
		u"p",

		u"[", //LayoutSpecific4
		u"]", //LayoutSpecific5

		u"\n",

		nullptr, //ControlLeft

		u"a",
		u"s",
		u"d",
		u"f",
		u"g",
		u"h",
		u"j",
		u"k",
		u"l",

		u";", //LayoutSpecific6
		u"'", //LayoutSpecific7
		u"`", //LayoutSpecific1

		nullptr, //ShiftLeft

		u"#", //LayoutSpecific8

		u"z",
		u"x",
		u"c",
		u"v",
		u"b",
		u"n",
		u"m",

		u",", //Comma
		u".", //Period

		u"/", //LayoutSpecific9

		nullptr, //ShiftRight
		u"*", //Multiply

		nullptr, //Alt
		u" ", //Space
		nullptr, //CapsLock

		nullptr, //Function1
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,

		nullptr, //NumLock
		nullptr, //ScrollLock

		u"7", //NumPad7
		u"8",
		u"9",
		u"-",
		u"4",
		u"5",
		u"6",
		u"+",
		u"1",
		u"2",
		u"3",
		u"0",
		u".",

		nullptr, //PrintScreen

		u"\\", //LayoutSpecific10

		nullptr, //Function11
		nullptr,
	},
	//Standard_Shift
	{
		nullptr,

		nullptr, //Escape
		u"!",
		u"\"",
		u"£",
		u"$",
		u"%",
		u"^",
		u"&",
		u"*",
		u"(",
		u")",

		u"_", //LayoutSpecific2
		u"+", //LayoutSpecific3
		nullptr, //Backspace
		nullptr, //Tab

		u"Q",
		u"W",
		u"E",
		u"R",
		u"T",
		u"Y",
		u"U",
		u"I",
		u"O",
		u"P",

		u"{", //LayoutSpecific4
		u"}", //LayoutSpecific5

		nullptr,

		nullptr, //ControlLeft

		u"A",
		u"S",
		u"D",
		u"F",
		u"G",
		u"H",
		u"J",
		u"K",
		u"L",

		u":", //LayoutSpecific6
		u"@", //LayoutSpecific7
		u"¬", //LayoutSpecific1

		nullptr, //ShiftLeft

		u"~", //LayoutSpecific8

		u"Z",
		u"X",
		u"C",
		u"V",
		u"B",
		u"N",
		u"M",

		u"<", //Comma
		u">", //Period

		u"?", //LayoutSpecific9

		nullptr, //ShiftRight
		nullptr, //Multiply

		nullptr, //Alt
		nullptr, //Space
		nullptr, //CapsLock

		nullptr, //Function1
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,

		nullptr, //NumLock
		nullptr, //ScrollLock

		nullptr, //NumPad7
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,

		nullptr, //PrintScreen

		u"|", //LayoutSpecific10

		nullptr, //Function11
		nullptr,
	},
	//Standard_AltGr
	{
		nullptr,

		nullptr, //Escape
		nullptr,
		nullptr,
		nullptr,
		u"€",
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,

		nullptr, //LayoutSpecific2
		nullptr, //LayoutSpecific3
		nullptr, //Backspace
		nullptr, //Tab

		nullptr,
		u"ẃ",
		u"é",
		nullptr,
		nullptr,
		u"ý",
		u"ú",
		u"í",
		u"ó",
		nullptr,

		nullptr, //LayoutSpecific4
		nullptr, //LayoutSpecific5

		nullptr,

		nullptr, //ControlLeft

		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,

		nullptr, //LayoutSpecific6
		nullptr, //LayoutSpecific7
		u"¦", //LayoutSpecific1

		nullptr, //ShiftLeft

		u"\\", //LayoutSpecific8

		nullptr,
		nullptr,
		u"ç",
		nullptr,
		nullptr,
		nullptr,
		nullptr,

		nullptr, //Comma
		nullptr, //Period

		nullptr, //LayoutSpecific9

		nullptr, //ShiftRight
		nullptr, //Multiply

		nullptr, //Alt
		nullptr, //Space
		nullptr, //CapsLock

		nullptr, //Function1
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,

		nullptr, //NumLock
		nullptr, //ScrollLock

		nullptr, //NumPad7
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,

		nullptr, //PrintScreen

		nullptr, //LayoutSpecific10

		nullptr, //Function11
		nullptr,
	},
	//Standard_ShiftAltGr
	{
		nullptr,

		nullptr, //Escape
		nullptr,
		nullptr,
		nullptr,
		u"€",
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,

		nullptr, //LayoutSpecific2
		nullptr, //LayoutSpecific3
		nullptr, //Backspace
		nullptr, //Tab

		nullptr,
		u"Ẃ",
		u"É",
		nullptr,
		nullptr,
		u"Ý",
		u"Ú",
		u"Í",
		u"Ó",
		nullptr,

		nullptr, //LayoutSpecific4
		nullptr, //LayoutSpecific5

		nullptr,

		nullptr, //ControlLeft

		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,

		nullptr, //LayoutSpecific6
		nullptr, //LayoutSpecific7
		nullptr, //LayoutSpecific1

		nullptr, //ShiftLeft

		u"\\", //LayoutSpecific8

		nullptr,
		nullptr,
		u"Ç",
		nullptr,
		nullptr,
		nullptr,
		nullptr,

		nullptr, //Comma
		nullptr, //Period

		nullptr, //LayoutSpecific9

		nullptr, //ShiftRight
		nullptr, //Multiply

		nullptr, //Alt
		nullptr, //Space
		nullptr, //CapsLock

		nullptr, //Function1
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,

		nullptr, //NumLock
		nullptr, //ScrollLock

		nullptr, //NumPad7
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,

		nullptr, //PrintScreen

		nullptr, //LayoutSpecific10

		nullptr, //Function11
		nullptr,
	}
};

KeyState PhysicalKeyState;

uint32_t KeyboardInterrupt(void* Context)
{
	uint64_t status = InPort(0x64);
	uint8_t ascii = '?';
	bool released = false;
	if(status & 0x1)
	{
		uint8_t scancode = InPort(0x60);
		if(scancode == 0xE0 || scancode == 0xE1)
		{
			PhysicalKeyState.ExtendedKeyState = scancode;
			return 0;
		}
		else
		{
			PhysicalKeyState.ExtendedKeyState = 0;
		}
		
		uint32_t LastScancode = (PhysicalKeyState.ExtendedKeyState << 8) + scancode;

		PhysicalKeyState.UpdateKeyState(LastScancode);

		KeyString ascii = PhysicalKeyState.GetPrintable(LastScancode, CurrentKeyboardLayout);
		if(ascii)
		{
			ConsolePrint(ascii);
		}

		bool released = PhysicalKeyState.IsReleased(LastScancode);

		if(released)
		{
			SerialPrint(u"Released scancode: 0x");
			LastScancode -= 0x80;
		}
		else
		{
			SerialPrint(u"Pressed Scancode: 0x");
		}
			
		char16_t Buffer[16];
		witoabuf(Buffer, (uint32_t)LastScancode, 16);
		SerialPrint(Buffer);
		SerialPrint(u"\r\n");
	}
}

void InitKeyboard()
{
	memset(&PhysicalKeyState, 0, sizeof(PhysicalKeyState));

	CurrentKeyboardLayout = &KeyboardLayout_British;

	SetInterruptHandler(0x21, KeyboardInterrupt, nullptr);
	SetIRQEnabled(0x1, true); //Keyboard IRQ
}