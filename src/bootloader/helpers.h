#pragma once

#include <c-efi.h>

#define ONE_SECOND 1000000
#define ERROR_CHECK(expression, message) CheckStatus( systemTable, (expression), message )

void Print(CEfiSystemTable* systemTable, CEfiChar16* message);
void CheckStatus(CEfiSystemTable* systemTable, CEfiStatus status, CEfiChar16* message);

void Halt(CEfiSystemTable* systemTable, CEfiStatus status, CEfiChar16* message);

int witoabuf(CEfiChar16* buffer, int value, int base);
