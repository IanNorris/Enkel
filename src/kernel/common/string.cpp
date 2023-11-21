#include "common/string.h"

size_t IndexOfWhitespace(const char* input, size_t offset)
{
	uint32_t index = offset;
	while (input[index] != '\0')
	{
		if (input[index] == ' ' || input[index] == '\t' || input[index] == '\r' || input[index] == '\n')
		{
			return index;
		}
		index++;
	}
	return -1;
}

size_t IndexOfWhitespace(const char16_t* input, size_t offset)
{
	uint32_t index = offset;
	while (input[index] != '\0')
	{
		if (input[index] == ' ' || input[index] == '\t' || input[index] == '\r' || input[index] == '\n')
		{
			return index;
		}
		index++;
	}
	return -1;
}

size_t IndexOfChar(const char* input, char c)
{
    uint32_t index = 0;
    while (input[index] != '\0')
    {
        if (input[index] == c)
        {
            return index;
        }
        index++;
    }
    return -1;
}

size_t IndexOfChar(const char16_t* input, char c)
{
	uint32_t index = 0;
	while (input[index] != '\0')
	{
		if (input[index] == c)
		{
			return index;
		}
		index++;
	}
	return -1;
}

size_t IndexOfSeparator(const char* input)
{
    uint32_t index = 0;
    while (input[index] != '\0')
    {
        if (input[index] == '\\' || input[index] == '/')
        {
            return index;
        }
        index++;
    }
    return -1;
}

size_t IndexOfSeparator(const char16_t* input)
{
	uint32_t index = 0;
	while (input[index] != '\0')
	{
		if (input[index] == '\\' || input[index] == '/')
		{
			return index;
		}
		index++;
	}
	return -1;
}

template<typename T>
int witoabuf(char16_t* buffer, T value, int base, int padDigits)
{
	const static char16_t characters[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	const T isNegative = value < 0 && base == 10;
	const T absValue = isNegative ? -value : value;

	int offset = 0;

	T currentValue = absValue;
	do
	{
		T nextValue = currentValue / base;
		T remainder = currentValue - (nextValue * base);

		buffer[offset++] = characters[remainder];

		currentValue = nextValue;
	} while (currentValue != 0);

	do
	{
		buffer[offset++] = '0';
	} while(padDigits > offset);

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
		char16_t temp = buffer[index];
		buffer[index] = buffer[swapIndex];
		buffer[swapIndex] = temp;
	}

	return offset;
}

template<typename T>
int witoabuf(char* buffer, T value, int base, int padDigits)
{
	const static char characters[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	const T isNegative = value < 0 && base == 10;
	const T absValue = isNegative ? -value : value;

	int offset = 0;

	T currentValue = absValue;
	do
	{
		T nextValue = currentValue / base;
		T remainder = currentValue - (nextValue * base);

		buffer[offset++] = characters[remainder];

		currentValue = nextValue;
	} while (currentValue != 0);

	do
	{
		buffer[offset++] = '0';
	} while(padDigits > offset);

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
		char16_t temp = buffer[index];
		buffer[index] = buffer[swapIndex];
		buffer[swapIndex] = temp;
	}

	return offset;
}

template int witoabuf(char16_t* buffer, int32_t value, int base, int padDigits);
template int witoabuf(char16_t* buffer, uint32_t value, int base, int padDigits);
template int witoabuf(char16_t* buffer, int64_t value, int base, int padDigits);
template int witoabuf(char16_t* buffer, uint64_t value, int base, int padDigits);

template int witoabuf(char* buffer, int32_t value, int base, int padDigits);
template int witoabuf(char* buffer, uint32_t value, int base, int padDigits);
template int witoabuf(char* buffer, int64_t value, int base, int padDigits);
template int witoabuf(char* buffer, uint64_t value, int base, int padDigits);

int ascii_to_wide(char16_t* bufferOut, const char* bufferIn, int bufferOutBytes)
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

size_t _strlen(const char* str)
{
	const char* s = str;
	while (*s)
		++s;
	return s - str;
}

size_t _strlen(const char16_t* str)
{
	const char16_t* s = str;
	while (*s)
		++s;
	return s - str;
}

int _strcmp(const char* str1, const char* str2)
{
	while (*str1 && (*str1 == *str2))
	{
		str1++;
		str2++;
	}
	return *(int*)str1 - *(int*)str2;
}

int _strcmp(const char16_t* str1, const char16_t* str2)
{
	while (*str1 && (*str1 == *str2))
	{
		str1++;
		str2++;
	}
	return *(int*)str1 - *(int*)str2;
}

int _stricmp(const char* str1, const char* str2)
{
	while (*str1 && (tolower(*str1) == tolower(*str2)))
	{
		str1++;
		str2++;
	}

	return tolower(*str1) - tolower(*str2);
}

int _stricmp(const char16_t* str1, const char16_t* str2)
{
	while (*str1 && (tolower(*str1) == tolower(*str2)))
	{
		str1++;
		str2++;
	}

	return tolower(*str1) - tolower(*str2);
}

int _strnicmp(const char* str1, const char* str2, size_t n)
{
	while (n && *str1 && (tolower(*str1) == tolower(*str2)))
	{
		str1++;
		str2++;
		n--;
	}

	if (n == 0)
	{
		return 0;
	}
	else
	{
		return tolower(*str1) - tolower(*str2);
	}
}

int _strnicmp(const char16_t* str1, const char16_t* str2, size_t n)
{
	while (n && *str1 && (tolower(*str1) == tolower(*str2)))
	{
		str1++;
		str2++;
		n--;
	}

	if (n == 0)
	{
		return 0;
	}
	else
	{
		return tolower(*str1) - tolower(*str2);
	}
}

char* _strcpy(char* dest, const char* src)
{
	char* save = dest;
	while ((*dest++ = *src++));
	return save;
}

char* _strncpy(char* dest, const char* src, size_t n)
{
	size_t i;
	for (i = 0; i < n && src[i] != '\0'; i++)
	{
		dest[i] = src[i];
	}
	for (; i < n; i++)
	{
		dest[i] = '\0';
	}
	return dest;
}

char16_t* _strcpy(char16_t* dest, const char16_t* src)
{
	char16_t* save = dest;
	while ((*dest++ = *src++));
	return save;
}

char16_t* _strncpy(char16_t* dest, const char16_t* src, size_t n)
{
	size_t i;
	for (i = 0; i < n && src[i] != u'\0'; i++)
		dest[i] = src[i];
	for (; i < n; i++)
		dest[i] = u'\0';
	return dest;
}

int _isalpha(int c)
{
	return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

int _isupper(int c)
{
	return (c >= 'A' && c <= 'Z');
}

int _islower(int c)
{
	return (c >= 'a' && c <= 'z');
}

int _tolower(int c)
{
	if (_isupper(c))
		return 'a' + (c - 'A');
	else
		return c;
}

int _toupper(int c)
{
	if (_islower(c))
		return 'A' + (c - 'a');
	else
		return c;
}
