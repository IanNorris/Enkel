#include "helpers.h"
#include "common/string.h"

#define HALT_ASSERT 0xBADC0DE

EFI_SYSTEM_TABLE* GSystemTable = nullptr;

void InitHelpers(EFI_SYSTEM_TABLE* systemTable)
{
	GSystemTable = systemTable;
}

void KERNEL_API Print(const char16_t* message)
{
	UEFI_CALL(GSystemTable->ConOut, OutputString, (CHAR16*)message);
}

void SetColor(int color)
{
	UEFI_CALL(GSystemTable->ConOut, SetAttribute, color);
}

void Halt(EFI_STATUS status, const char16_t* message)
{
	if (status != HALT_ASSERT)
	{
		SetColor(EFI_BACKGROUND_RED | EFI_WHITE);
		Print(u"\r\n\r\n ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ");
		Print(u"\r\n ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ");
		SetColor(EFI_YELLOW);

		Print(u"\r\nFATAL ERROR: ");

		char16_t buffer[16];
		witoabuf<int>(buffer, status, 16);

		SetColor(EFI_WHITE);

		Print(u"(0x");
		Print(buffer);
		Print(u") ");
		Print(message);

		SetColor(EFI_BACKGROUND_RED | EFI_WHITE);

		Print(u"\r\n ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ");
		Print(u"\r\n ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ");

		SetColor(EFI_WHITE);
	}

	while (1)
	{
		GSystemTable->BootServices->Stall(ONE_SECOND);
	}
}

void CheckStatus(EFI_STATUS status, const char16_t* message)
{
	if (status != EFI_SUCCESS)
	{
		Halt(status, message);
	}
}

extern "C" void AssertionFailed(const char* expression, const char* message, const char* filename, size_t lineNumber)
{
	char16_t buffer[2048];

	SetColor(EFI_BACKGROUND_RED | EFI_WHITE);
	Print(u"\r\n\r\n__________ ASSERTION FAILED __________");
	SetColor(EFI_YELLOW);
	
	ascii_to_wide(buffer, expression, sizeof(buffer));
	SetColor(EFI_YELLOW);
	Print(u"\r\nEXPRESSION: ");
	SetColor(EFI_WHITE);
	Print(buffer);

	SetColor(EFI_YELLOW);
	Print(u"\r\nFILE:       ");
	ascii_to_wide(buffer, filename, sizeof(buffer));
	SetColor(EFI_WHITE);
	Print(buffer);

	SetColor(EFI_YELLOW);
	Print(u"\r\nLINE:       ");
	witoabuf(buffer, lineNumber, 10);
	SetColor(EFI_WHITE);
	Print(buffer);

	SetColor(EFI_YELLOW);
	Print(u"\r\nMESSAGE:    ");
	ascii_to_wide(buffer, message, sizeof(buffer));
	SetColor(EFI_WHITE);
	Print(buffer);

	SetColor(EFI_BACKGROUND_RED | EFI_WHITE);
	Print(u"\r\n__________ ASSERTION FAILED __________");

	Halt(HALT_ASSERT, nullptr);
}
