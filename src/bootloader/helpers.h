#pragma once

#define _Bool bool
#include <c-efi.h>

#define ONE_SECOND 1000000
#define ERROR_CHECK(expression, message) CheckStatus( (expression), message )

void InitHelpers(CEfiSystemTable* systemTable);
void Print(const char16_t* message);
void CheckStatus(CEfiStatus status, const char16_t* message);

void Halt(CEfiStatus status, const char16_t* message);
