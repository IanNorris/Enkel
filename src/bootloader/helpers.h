#pragma once

#undef NULL
#define EFIAPI
#define IN
#define OUT
#define UINT64 uint64_t
#include "Uefi.h"
#include "Library/UefiBootServicesTableLib.h"
#include "Protocol/SimpleFileSystem.h"

#define ONE_SECOND 1000000
#define ERROR_CHECK(expression, message) CheckStatus( (expression), message )
#define UEFI_CALL(target, functionName, ...) ERROR_CHECK(target->functionName(target, ##__VA_ARGS__), u ## #target)

void InitHelpers(EFI_SYSTEM_TABLE* systemTable);
void Print(const char16_t* message);
void SetColor(int color);
void CheckStatus(EFI_STATUS status, const char16_t* message);

void Halt(EFI_STATUS status, const char16_t* message);
