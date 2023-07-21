#pragma once

#include "common/types.h"

size_t IndexOfChar(const char* input, char c);
size_t IndexOfSeparator(const char* input);
int witoabuf(char16_t* buffer, int value, int base);
int ascii_to_wide(char16_t* bufferOut, const char* bufferIn, int bufferOutBytes);

int _isalpha(int c);
int _isupper(int c);
int _islower(int c);
int _tolower(int c);
int _toupper(int c);

#define isalpha _isalpha
#define isupper _isupper
#define islower _islower
#define tolower _tolower
#define toupper _toupper

size_t _strlen(const char* str);
int _strcmp(const char* str1, const char* str2);
int _stricmp(const char* str1, const char* str2);
int _strnicmp(const char* str1, const char* str2, size_t n);

#define strlen _strlen
#define _strncasecmp _strnicmp
