#pragma once

#include "common/types.h"
#include "kernel/framebuffer/framebuffer.h"
#include "font/details/bmfont.h"

#define CONSOLE_MAX_CHARS (64 * 1024)
#define CONSOLE_MAX_ROWS 100

struct Console
{
	FramebufferLayout Framebuffer;
	const BMFont* Font;
	uint32_t Background;
	BMFontColor Foreground;
	uint32_t CurrentX;
	uint32_t CurrentY;
	uint32_t ReturnX;

	uint32_t CharacterHeight;

	uint32_t ScreenRows;

	char16_t CharacterBuffer[CONSOLE_MAX_CHARS];
	char16_t* RowIndices[CONSOLE_MAX_ROWS];
	uint32_t RowLength[CONSOLE_MAX_ROWS];
	uint32_t RowOrder[CONSOLE_MAX_ROWS];
	uint32_t CurrentRow;
};

void SetSerialTargetBuffer(char* NewTargetBuffer);

void SerialPrint(const char* String);
void SerialPrint(const char16_t* String);

void HexDump(const uint8_t* Bytes, uint32_t ByteLength, uint32_t BytesPerLine = 32);

const BMFont* GetFont(Console* Console = nullptr);

void DefaultFontInit();
void ConsoleInit(Console* Console, const BMFont* Font, const FramebufferLayout& Framebuffer, BMFontColor BackgroundColor, BMFontColor ForegroundColor);

void DefaultConsoleInit(const FramebufferLayout& Framebuffer, BMFontColor BackgroundColor, BMFontColor ForegroundColor);

void ConsolePrintAtPosWithColor(const char16_t* String, int32_t& X, int32_t& Y, int32_t ReturnX, BMFontColor ForegroundColor, Console* Console = nullptr);
void ConsolePrintAtPos(const char16_t* String, int32_t& X, int32_t& Y, int32_t ReturnX, Console* Console = nullptr);
void ConsolePrint(const char16_t* String, Console* Console = nullptr);

void ConsolePrintNumeric(const char16_t* Start, uint64_t Value, const char16_t* Suffix, int Base = 16);
void LogPrintNumeric(const char16_t* Start, uint64_t Value, const char16_t* Suffix, int Base = 16);

void ConsoleSetPos(int32_t x, int32_t y, Console* Console = nullptr);

#if VERBOSE_LOGGING
#define VerboseLog(x) ConsolePrint(x)
#define Verbose(x) x
#else
#define VerboseLog(x)
#define Verbose(x)
#endif