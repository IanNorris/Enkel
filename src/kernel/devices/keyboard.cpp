#include "kernel/console/console.h"
#include "kernel/devices/keyboard.h"
#include "kernel/init/interrupts.h"
#include "kernel/init/msr.h"
#include "kernel/init/pic.h"
#include "common/string.h"
#include "memory/memory.h"
#include "fs/volumes/stdio.h"

KeyLayout* CurrentKeyboardLayout;

extern KeyLayout KeyboardLayout_British;

KeyState PhysicalKeyState;

uint32_t KeyboardInterrupt(void* Context)
{
	uint64_t status = InPort(0x64);
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
		if (ascii)
		{
			ConsolePrint(ascii);
			KeyString asciiCopy = ascii;
			while (*asciiCopy)
			{
				InsertInput((char)*(asciiCopy++), false);
			}
		}

		InsertInput(scancode, true);
	}
}

void InitKeyboard()
{
	memset(&PhysicalKeyState, 0, sizeof(PhysicalKeyState));

	CurrentKeyboardLayout = &KeyboardLayout_British;

	SetInterruptHandler(0x21, KeyboardInterrupt, nullptr);
	SetIRQEnabled(0x1, true); //Keyboard IRQ
}