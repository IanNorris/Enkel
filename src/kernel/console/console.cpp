#include "kernel/console/console.h"

#include "CascadiaMono.h"
#include "CascadiaMono_0.h"
#include "common/string.h"
#include "font/details/bmfont_internal.h"
#include "font/details/bmfont.h"
#include "memory/memory.h"
#include "kernel/init/msr.h"

BMFont GDefaultFont;
Console GDefaultConsole;

bool EnableSerialOutput = false;

char* TargetBuffer = nullptr;

void SetSerialTargetBuffer(char* NewTargetBuffer)
{
	TargetBuffer = NewTargetBuffer;
}

void InitializeSerial0()
{
	TargetBuffer = nullptr;

	const uint16_t port = 0x3F8;
    OutPort(port + 1, 0x00);  // Disable all interrupts
    OutPort(port + 3, 0x80);  // Enable DLAB (set baud rate divisor)
    OutPort(port + 0, 0x03);  // Set divisor to 3 (low byte) 38400 baud
    OutPort(port + 1, 0x00);  // (high byte)
    OutPort(port + 3, 0x03);  // 8 bits, no parity, one stop bit

	// Read back line control register to verify
    uint8_t lcr = InPort(port + 3);
    if (lcr == 0x03)
	{
		EnableSerialOutput = true;
	}
	else
	{
		EnableSerialOutput = false;
		ConsolePrint(u"COM1 not detected.\n");
	}

	OutPort(port + 4, 0x0F); // Disable loopback

	OutPort(port + 2, 0x07); //Flush FIFO queues
}

uint8_t LastChar = 0xFF;
int TotalBytesWritten = 0;

void WriteSerial0Int(uint8_t character)
{
	if(EnableSerialOutput)
	{
		const uint16_t port = 0x3F8;

		//Wait until we've transmitted
		while ((InPort(port + 5) & 0x20) == 0)
		{
			asm("pause");
		}

		OutPort(port, character);
	}

	LastChar = character;
	TotalBytesWritten++;

	if(TargetBuffer)
	{
		*TargetBuffer++ = character;
		*TargetBuffer = '\0';
	}
}

void WriteSerial0(uint8_t character)
{
	if(character == '\n')
	{
		WriteSerial0Int('\r');
		WriteSerial0Int('\n');
	}
	else if(character == '\r')
	{
		
	}
	else
	{
		WriteSerial0Int(character);
	}
}

void SerialPrint(const char16_t* String)
{
	const char16_t* ToConsole = String;
	while(*ToConsole)
	{
		WriteSerial0((char)*ToConsole);

		ToConsole++;
	}
}

void SerialPrint(const char* String)
{
	while(*String)
	{
		WriteSerial0((char)*String);

		String++;
	}
}

void HexDump(const uint8_t* Bytes, uint32_t ByteLength, uint32_t BytesPerLine)
{
	char16_t Buffer[16];

	for(uint32_t i = 0; i < ByteLength; i++)
	{
		if(i % BytesPerLine == 0)
		{
			ConsolePrint(u"\n");
		}

		witoabuf<uint32_t>(Buffer, Bytes[i], 16, 2);
		ConsolePrint(Buffer);
		ConsolePrint(u" ");
	}
	ConsolePrint(u"\n");
}

const BMFont* GetFont(Console* Console)
{
	if (Console == nullptr)
	{
		Console = &GDefaultConsole;
	}

	return Console->Font;
}

void DefaultFontInit()
{
    BMFont_Load(CascadiaMono_fnt_data, CascadiaMono_fnt_size, &GDefaultFont);
    GDefaultFont.PageTextureData[0] = CascadiaMono_0_tga_data;
    GDefaultFont.PageTextureSize[0] = CascadiaMono_0_tga_size;
}

void ConsoleClear(Console* Console)
{
	if (Console == nullptr)
	{
		Console = &GDefaultConsole;
	}

	Console->CurrentRow = 0;
	Console->ReturnX = 0;
	memset(Console->CharacterBuffer, 0, sizeof(Console->CharacterBuffer[0]) * CONSOLE_MAX_CHARS);
	memset(Console->RowIndices, 0, sizeof(Console->RowIndices[0]) * CONSOLE_MAX_ROWS);
	memset(Console->RowLength, 0, sizeof(Console->RowLength[0]) * CONSOLE_MAX_ROWS);
	memset(Console->RowOrder, 0, sizeof(Console->RowOrder[0]) * CONSOLE_MAX_ROWS);
	memset32(Console->Framebuffer.Base, Console->Background, Console->Framebuffer.Pitch * Console->Framebuffer.Height);
}

void ConsoleInit(Console* Console, const BMFont* Font, const FramebufferLayout& Framebuffer, BMFontColor BackgroundColor, BMFontColor ForegroundColor)
{
	uint32_t Background = BackgroundColor.Blue | (BackgroundColor.Green << 8) | (BackgroundColor.Red << 16);

	Console->Framebuffer = Framebuffer;
	Console->Font = Font;
	Console->Background = Background;
	Console->Foreground = ForegroundColor;
	Console->CurrentX = 0;
	Console->CurrentY = 0;
	Console->ReturnX = 0;
	Console->CharacterHeight = Console->Font->Common->LineHeight;
	Console->ScreenRows = Console->Framebuffer.Height / Console->CharacterHeight;
	if (Console->ScreenRows > CONSOLE_MAX_ROWS)
	{
		Console->ScreenRows = CONSOLE_MAX_ROWS;
	}

	ConsoleClear(Console);

	InitializeSerial0();
}

void DefaultConsoleInit(const FramebufferLayout& Framebuffer, BMFontColor BackgroundColor, BMFontColor ForegroundColor)
{
	DefaultFontInit();
    ConsoleInit(&GDefaultConsole, &GDefaultFont, Framebuffer, BackgroundColor, ForegroundColor);
}

void ConsolePrintAtPosWithColor(const char16_t* String, int32_t& X, int32_t& Y, int32_t ReturnX, BMFontColor ForegroundColor, Console* Console)
{
	if (Console == nullptr)
	{
		Console = &GDefaultConsole;
	}

	if (X == -1)
	{
		X = Console->CurrentX;
	}

	if (Y == -1)
	{
		Y = Console->CurrentY;
	}

	if (ReturnX == -1)
	{
		ReturnX = Console->ReturnX;
	}

	BMFont_Render(Console->Font, &Console->Framebuffer, X, Y, ReturnX, String, ForegroundColor);
}

void ConsolePrintAtPos(const char16_t* String, int32_t& X, int32_t& Y, int32_t ReturnX, Console* Console)
{
	if (Console == nullptr)
	{
		Console = &GDefaultConsole;
	}

	if (X == -1)
	{
		X = Console->CurrentX;
	}

	if (Y == -1)
	{
		Y = Console->CurrentY;
	}

	if (ReturnX == -1)
	{
		ReturnX = Console->ReturnX;
	}

	ConsolePrintAtPosWithColor(String, X, Y, ReturnX, Console->Foreground, Console);
}

void ConsolePrint(const char16_t* String, Console* Console)
{
	SerialPrint(String);

	if (Console == nullptr)
	{
		Console = &GDefaultConsole;
	}

	if (Console->CurrentY + Console->Font->Common->LineHeight > Console->Framebuffer.Height)
	{
		Console->ReturnX += 400;

		Console->CurrentY = 0;
		Console->CurrentX = Console->ReturnX;
	}

	int32_t X = Console->CurrentX;
	int32_t Y = Console->CurrentY;
	ConsolePrintAtPos(String, X, Y, Console->ReturnX, Console);

	Console->CurrentX = X;
	Console->CurrentY = Y;
}

void ConsolePrintNumeric(const char16_t* Start, uint64_t Value, const char16_t* Suffix, int Base)
{
	char16_t Buffer[32];

	ConsolePrint(Start);
	if(Base == 16)
	{
		ConsolePrint(u"0x");
	}
	witoabuf(Buffer, Value, Base);
	ConsolePrint(Buffer);
	ConsolePrint(Suffix);
}

void LogPrintNumeric(const char16_t* Start, uint64_t Value, const char16_t* Suffix, int Base)
{
	char16_t Buffer[32];

	SerialPrint(Start);
	if(Base == 16)
	{
		SerialPrint(u"0x");
	}
	witoabuf(Buffer, Value, Base);
	SerialPrint(Buffer);
	SerialPrint(Suffix);
}

void ConsoleSetPos(int32_t x, int32_t y, Console* Console)
{
	if (Console == nullptr)
	{
		Console = &GDefaultConsole;
	}

	Console->CurrentX = x;
	Console->CurrentY = y;
}
