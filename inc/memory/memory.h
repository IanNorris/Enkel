#pragma once

#include "common/types.h"

void* memcpy(void* dest, const void* src, size_t n);
volatile void* memcpy(volatile void* dest, const void* src, size_t n);
void* memset(void* s, uint8_t c, size_t n);
volatile void* memset(volatile void* s, uint8_t c, size_t n);
void* memset32(void* s, uint32_t value, size_t n);
void* memset64(void* s, uint64_t value, size_t n);
void* memmove(void* dest, const void* src, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
void FillUnique(void* address, uint64_t baseValue, uint64_t byteCount);
