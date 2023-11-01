#include "kernel/console/console.h"
#include "kernel/init/interrupts.h"
#include "kernel/init/msr.h"
#include "kernel/init/pic.h"


char ScancodeToAscii[] =
{
	0, 0,
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
	'\b', 
	'\t',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
	'\n',
	0, //ctrl
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '#',
	0, //shift
	'\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
	0, //shift right
	0, //print screen
	0, //alt,
	' ',
	0, //caps lock
};

uint32_t KeyboardInterrupt(void* Context)
{
	uint64_t status = InPort(0x64);
	uint8_t ascii = '?';
	bool released = false;
	if(status & 0x1)
	{
		uint8_t scancode = InPort(0x60);
		if(scancode < sizeof(ScancodeToAscii)/sizeof(ScancodeToAscii[0]))
		{
			ascii = ScancodeToAscii[scancode];
		}
		else
		{
			scancode -= 0x80;

			if(scancode < sizeof(ScancodeToAscii)/sizeof(ScancodeToAscii[0]))
			{
				ascii = ScancodeToAscii[scancode];
				released = true;
			}
			else
			{
				ascii = '@';
			}
		}

		if(ascii == 0)
		{
			ascii = '?';
		}

		if(!released)
		{
			char16_t buffer[2];
			buffer[0] = ascii;
			buffer[1] = '\0';
			ConsolePrint(buffer);
		}
	}
}

void InitKeyboard()
{
	SetInterruptHandler(0x21, KeyboardInterrupt, nullptr);
	SetIRQEnabled(0x1, true); //Keyboard IRQ
}