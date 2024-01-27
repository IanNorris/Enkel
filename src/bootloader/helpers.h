#pragma once

#include <common/types.h>

#undef NULL
#define EFIAPI
#define IN
#define OUT
#define UINT64 uint64_t
#include "Uefi.h"
#undef FALSE
#undef TRUE
#include "Library/UefiBootServicesTableLib.h"
#include "Protocol/SimpleFileSystem.h"

struct Allocation
{
	EFI_PHYSICAL_ADDRESS Data;
	uint64_t PageCount;
};

#define ONE_SECOND 1000000
#define ERROR_CHECK(expression, message) CheckStatus( (expression), message )
#define UEFI_CALL(target, functionName, ...) ERROR_CHECK(target->functionName(target, ##__VA_ARGS__), u ## #target)

void InitHelpers(EFI_SYSTEM_TABLE* systemTable);
void KERNEL_API Print(const char16_t* message);
void SetColor(int color);
void CheckStatus(EFI_STATUS status, const char16_t* message);

void Halt(EFI_STATUS status, const char16_t* message);

bool CompareGuids(const EFI_GUID& Guid1, const EFI_GUID& Guid2);

Allocation AllocatePages(uint32_t PageType, uint64_t SizeInBytes);
void FreePages(Allocation& Allocation);
