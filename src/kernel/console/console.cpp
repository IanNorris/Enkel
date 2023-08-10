#include "kernel/console/console.h"

#include "Jura.h"
#include "Jura_0.h"
#include "common/string.h"
#include "font/details/bmfont_internal.h"
#include "font/details/bmfont.h"
#include "memory/memory.h"

BMFont GDefaultFont;
Console GDefaultConsole;

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
    BMFont_Load(Jura_fnt_data, Jura_fnt_size, &GDefaultFont);
    GDefaultFont.PageTextureData[0] = Jura_0_tga_data;
    GDefaultFont.PageTextureSize[0] = Jura_0_tga_size;
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

	BMFont_Render(Console->Font, &Console->Framebuffer, X, Y, ReturnX, String, ForegroundColor);
}

void ConsolePrintAtPos(const char16_t* String, int32_t& X, int32_t& Y, int32_t ReturnX, Console* Console)
{
	if (Console == nullptr)
	{
		Console = &GDefaultConsole;
	}

	ConsolePrintAtPosWithColor(String, X, Y, ReturnX, Console->Foreground, Console);
}

void ConsolePrint(const char16_t* String, Console* Console)
{
	if (Console == nullptr)
	{
		Console = &GDefaultConsole;
	}

	if (Console->CurrentY + Console->Font->Common->LineHeight > Console->Framebuffer.Height)
	{
		Console->ReturnX += 600;

		Console->CurrentY = 0;
		Console->CurrentX = Console->ReturnX;
	}

	int32_t X = Console->CurrentX;
	int32_t Y = Console->CurrentY;
	ConsolePrintAtPos(String, X, Y, Console->ReturnX, Console);

	Console->CurrentX = X;
	Console->CurrentY = Y;
}
