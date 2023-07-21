#include "helpers.h"
#include "common/string.h"

#define HALT_ASSERT 0xBADC0DE

EFI_SYSTEM_TABLE* GSystemTable = nullptr;

void InitHelpers(EFI_SYSTEM_TABLE* systemTable)
{
	GSystemTable = systemTable;
}

void Print(const char16_t* message)
{
	GSystemTable->ConOut->OutputString(GSystemTable->ConOut, (CHAR16*)message);
}

void Halt(EFI_STATUS status, const char16_t* message)
{
	if (status != HALT_ASSERT)
	{
		GSystemTable->ConOut->SetAttribute(GSystemTable->ConOut, EFI_BACKGROUND_RED | EFI_WHITE);

		Print(u"\r\n\r\n ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ");
		Print(u"\r\n ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ");

		GSystemTable->ConOut->SetAttribute(GSystemTable->ConOut, EFI_YELLOW);

		Print(u"\r\nFATAL ERROR: ");

		char16_t buffer[16];
		witoabuf(buffer, status, 16);

		GSystemTable->ConOut->SetAttribute(GSystemTable->ConOut, EFI_WHITE);

		Print(u"(0x");
		Print(buffer);
		Print(u") ");
		Print(message);

		GSystemTable->ConOut->SetAttribute(GSystemTable->ConOut, EFI_BACKGROUND_RED | EFI_WHITE);

		Print(u"\r\n ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ");
		Print(u"\r\n ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ");

		GSystemTable->ConOut->SetAttribute(GSystemTable->ConOut, EFI_WHITE);
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

void AssertionFailed(const char* expression, const char* message, const char* filename, size_t lineNumber)
{
	char16_t buffer[2048];

	GSystemTable->ConOut->SetAttribute(GSystemTable->ConOut, EFI_BACKGROUND_RED | EFI_WHITE);
	Print(u"\r\n\r\n__________ ASSERTION FAILED __________");
	GSystemTable->ConOut->SetAttribute(GSystemTable->ConOut, EFI_YELLOW);
	
	ascii_to_wide(buffer, expression, sizeof(buffer));
	GSystemTable->ConOut->SetAttribute(GSystemTable->ConOut, EFI_YELLOW);
	Print(u"\r\nEXPRESSION: ");
	GSystemTable->ConOut->SetAttribute(GSystemTable->ConOut, EFI_WHITE);
	Print(buffer);

	GSystemTable->ConOut->SetAttribute(GSystemTable->ConOut, EFI_YELLOW);
	Print(u"\r\nFILE:       ");
	ascii_to_wide(buffer, filename, sizeof(buffer));
	GSystemTable->ConOut->SetAttribute(GSystemTable->ConOut, EFI_WHITE);
	Print(buffer);

	GSystemTable->ConOut->SetAttribute(GSystemTable->ConOut, EFI_YELLOW);
	Print(u"\r\nLINE:       ");
	witoabuf(buffer, lineNumber, 10);
	GSystemTable->ConOut->SetAttribute(GSystemTable->ConOut, EFI_WHITE);
	Print(buffer);

	GSystemTable->ConOut->SetAttribute(GSystemTable->ConOut, EFI_YELLOW);
	Print(u"\r\nMESSAGE:    ");
	ascii_to_wide(buffer, message, sizeof(buffer));
	GSystemTable->ConOut->SetAttribute(GSystemTable->ConOut, EFI_WHITE);
	Print(buffer);

	GSystemTable->ConOut->SetAttribute(GSystemTable->ConOut, EFI_BACKGROUND_RED | EFI_WHITE);
	Print(u"\r\n__________ ASSERTION FAILED __________");

	Halt(HALT_ASSERT, nullptr);
}
