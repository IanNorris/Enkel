#include "helpers.h"
#include "common/string.h"

#define HALT_ASSERT 0xBADC0DE

CEfiSystemTable* GSystemTable = nullptr;

void InitHelpers(CEfiSystemTable* systemTable)
{
	GSystemTable = systemTable;
}

void Print(const char16_t* message)
{
	GSystemTable->con_out->output_string(GSystemTable->con_out, (CEfiChar16*)message);
}

void Halt(CEfiStatus status, const char16_t* message)
{
	if (status != HALT_ASSERT)
	{
		GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_BACKGROUND_RED | C_EFI_WHITE);

		Print(u"\r\n\r\n ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ");
		Print(u"\r\n ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ");

		GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_YELLOW);

		Print(u"\r\nFATAL ERROR: ");

		char16_t buffer[16];
		witoabuf(buffer, status, 16);

		GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_WHITE);

		Print(u"(0x");
		Print(buffer);
		Print(u") ");
		Print(message);

		GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_BACKGROUND_RED | C_EFI_WHITE);

		Print(u"\r\n ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ");
		Print(u"\r\n ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ");

		GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_WHITE);
	}

	while (1)
	{
		GSystemTable->boot_services->stall(ONE_SECOND);
	}
}

void CheckStatus(CEfiStatus status, const char16_t* message)
{
	if (status != C_EFI_SUCCESS)
	{
		Halt(status, message);
	}
}

void AssertionFailed(const char* expression, const char* message, const char* filename, size_t lineNumber)
{
	char16_t buffer[2048];

	GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_BACKGROUND_RED | C_EFI_WHITE);
	Print(u"\r\n\r\n__________ ASSERTION FAILED __________");
	GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_YELLOW);
	
	ascii_to_wide(buffer, expression, sizeof(buffer));
	GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_YELLOW);
	Print(u"\r\nEXPRESSION: ");
	GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_WHITE);
	Print(buffer);

	GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_YELLOW);
	Print(u"\r\nFILE:       ");
	ascii_to_wide(buffer, filename, sizeof(buffer));
	GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_WHITE);
	Print(buffer);

	GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_YELLOW);
	Print(u"\r\nLINE:       ");
	witoabuf(buffer, lineNumber, 10);
	GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_WHITE);
	Print(buffer);

	GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_YELLOW);
	Print(u"\r\nMESSAGE:    ");
	ascii_to_wide(buffer, message, sizeof(buffer));
	GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_WHITE);
	Print(buffer);

	GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_BACKGROUND_RED | C_EFI_WHITE);
	Print(u"\r\n__________ ASSERTION FAILED __________");

	Halt(HALT_ASSERT, nullptr);
}
