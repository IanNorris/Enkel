#include "helpers.h"

int witoabuf(CEfiChar16* buffer, int value, int base)
{
	const static CEfiChar16 characters[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	const int isNegative = value < 0 && base == 10;
	const unsigned int absValue = isNegative ? -value : (unsigned int)value;

	int offset = 0;

	unsigned int currentValue = absValue;
	do
	{
		unsigned int nextValue = currentValue / base;
		unsigned int remainder = currentValue - (nextValue * base);

		buffer[offset++] = characters[remainder];

		currentValue = nextValue;
	} while (currentValue != 0);

	if (isNegative)
	{
		buffer[offset++] = '-';
	}

	buffer[offset] = '\0';

	// String is now constructed backwards. Flipping it is very likely 
	// faster than calculating the exact string length and building backwards.
	// If this was in a hot path I'd profile it to check.

	int halfOffset = (offset - 1) / 2;

	for (int index = 0; index <= halfOffset; index++)
	{
		int swapIndex = offset - index - 1;
		CEfiChar16 temp = buffer[index];
		buffer[index] = buffer[swapIndex];
		buffer[swapIndex] = temp;
	}

	return offset;
}

int ascii_to_wide(CEfiChar16* bufferOut, const char* bufferIn, int bufferOutBytes)
{
	int index = 0;
	while (bufferIn[index] != '\0' && index < bufferOutBytes)
	{
		bufferOut[index] = bufferIn[index];
		index++;
	}
	bufferOut[index] = '\0';

	return index;
}

void Print(CEfiSystemTable* systemTable, CEfiChar16* message)
{
	systemTable->con_out->output_string(systemTable->con_out, message);
}

void Halt(CEfiSystemTable* systemTable, CEfiStatus status, CEfiChar16* message)
{
	systemTable->con_out->set_attribute(systemTable->con_out, C_EFI_RED);
	Print(systemTable, L"\r\nFATAL ERROR: ");
	systemTable->con_out->set_attribute(systemTable->con_out, C_EFI_WHITE);

	CEfiChar16 buffer[16];
	witoabuf(buffer, status, 16);

	Print(systemTable, L"(0x");
	Print(systemTable, buffer);
	Print(systemTable, L") ");
	Print(systemTable, message);

	while (1)
	{
		systemTable->boot_services->stall(ONE_SECOND);
	}
}

void CheckStatus(CEfiSystemTable* systemTable, CEfiStatus status, CEfiChar16* message)
{
	if (status != C_EFI_SUCCESS)
	{
		Halt(systemTable, status, message);
	}
}