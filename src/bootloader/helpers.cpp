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

extern "C" void AssertionFailed(const char* expression, const char* message, const char* filename, size_t lineNumber, uint64_t v1, uint64_t v2, uint64_t v3)
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

void __attribute__((used,noinline)) DebuggerHook()
{
    //Need some instructions to insert an interrupt into
    asm volatile("nop;nop;nop;nop;");
}

void __attribute__((used, noinline, noreturn)) HaltPermanently(void)
{
    //Stop interrupts
    asm("cli");
    while(true)
    {
        asm("hlt");
    }
}

bool CompareGuids(const EFI_GUID& Guid1, const EFI_GUID& Guid2)
{
	if(!(Guid1.Data1 == Guid2.Data1
		&& Guid1.Data2 == Guid2.Data2 
		&& Guid1.Data3 == Guid2.Data3))
	{
		return false;
	}

	for(int part = 0; part < 8; part++)
	{
		if(Guid1.Data4[part] != Guid2.Data4[part])
		{
			return false;
		}
	}

	return true;
}

Allocation AllocatePages(uint32_t PageType, uint64_t SizeInBytes)
{
	Allocation Out;
	Out.PageCount = (SizeInBytes + (EFI_PAGE_SIZE-1)) / EFI_PAGE_SIZE;

	ERROR_CHECK(GSystemTable->BootServices->AllocatePages(AllocateAnyPages, (EFI_MEMORY_TYPE)PageType, Out.PageCount, &Out.Data), u"AllocatePages failed");
	return Out;
}

void FreePages(Allocation& Allocation)
{
	ERROR_CHECK(GSystemTable->BootServices->FreePages(Allocation.Data, Allocation.PageCount), u"FreePages failed");
	Allocation.Data = 0;
}
