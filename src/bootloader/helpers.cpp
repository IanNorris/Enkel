#include "helpers.h"
#include "common/string.h"

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
	GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_RED);
	Print(u"\r\nFATAL ERROR: ");
	GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_WHITE);

	char16_t buffer[16];
	witoabuf(buffer, status, 16);

	Print(u"(0x");
	Print(buffer);
	Print(u") ");
	Print(message);

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

	GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_RED);
	Print(u"\r\n--------------------------------------------");
	Print(u"\r\nASSERTION FAILED: ");
	Print(u"\r\n--------------------------------------------");

	ascii_to_wide(buffer, expression, sizeof(buffer));
	Print(buffer);
	GSystemTable->con_out->set_attribute(GSystemTable->con_out, C_EFI_WHITE);

	Print(u"\r\nMESSAGE: ");
	ascii_to_wide(buffer, message, sizeof(buffer));
	Print(buffer);

	Print(u"\r\nFILE: ");
	ascii_to_wide(buffer, filename, sizeof(buffer));
	Print(buffer);

	Print(u"\r\nLINE: ");
	witoabuf(buffer, lineNumber, 10);
	Print(buffer);

	Print(u"\r\n\r\n");
	Halt(C_EFI_ABORTED, u"ASSERTED");
}
