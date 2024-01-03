#pragma once

#include "common/types.h"

size_t IndexOfWhitespace(const char* input, size_t offset);
size_t IndexOfChar(const char* input, char c);
size_t IndexOfSeparator(const char* input);

size_t IndexOfWhitespace(const char16_t* input, size_t offset);
size_t IndexOfChar(const char16_t* input, char16_t c);
size_t IndexOfSeparator(const char16_t* input);

template<typename T>
int witoabuf(char16_t* buffer, T value, int base, int padDigits = 0);

template<typename T>
int witoabuf(char* buffer, T value, int base, int padDigits = 0);

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
size_t _strlen(const char16_t* str);
int _strcmp(const char* str1, const char* str2);
int _strcmp(const char16_t* str1, const char16_t* str2);
int _stricmp(const char* str1, const char* str2);
int _stricmp(const char16_t* str1, const char16_t* str2);
int _strnicmp(const char* str1, const char* str2, size_t n);
int _strnicmp(const char16_t* str1, const char16_t* str2, size_t n);

char* _strcpy(char* dest, const char* src);
char* _strncpy(char* dest, const char* src, size_t n);
char16_t* _strcpy(char16_t* dest, const char16_t* src);
char16_t* _strncpy(char16_t* dest, const char16_t* src, size_t n);
char* _strcat(char* dest, const char* src);
char16_t* _strcat(char16_t* dest, const char16_t* src);

#define strlen _strlen
#define strcmp _strcmp
#define stricmp _stricmp
#define _strncasecmp _strnicmp
#define strcpy _strcpy
#define strncpy _strncpy
#define strcat _strcat
