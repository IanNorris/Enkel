#include "common/types.h"

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void* memcpy(void* dest, const void* src, size_t n) 
{
    uint8_t* pdest = (uint8_t*)dest;
    const uint8_t* psrc = (const uint8_t*)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

volatile void* memcpy(volatile void* dest, const void* src, size_t n) 
{
    volatile uint8_t* pdest = (volatile uint8_t*)dest;
    const uint8_t* psrc = (const uint8_t*)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void* memset(void* s, uint8_t value, size_t n)
{
    uint8_t* p = (uint8_t*)s;

    for (size_t i = 0; i < n; i++)
    {
        p[i] = value;
    }

    return s;
}

volatile void* memset(volatile void* s, uint8_t value, size_t n)
{
    volatile uint8_t* p = (volatile uint8_t*)s;

    for (size_t i = 0; i < n; i++)
    {
        p[i] = value;
    }

    return s;
}

void* memset32(void* s, uint32_t value, size_t n)
{
    uint32_t* p = (uint32_t*)s;

    size_t count = n / sizeof(value);
    for (size_t i = 0; i < count; i++)
    {
        p[i] = value;
    }

    return s;
}

void* memset64(void* s, uint64_t value, size_t n)
{
    uint64_t* p = (uint64_t*)s;

    size_t count = n / sizeof(value);
    for (size_t i = 0; i < count; i++)
    {
        p[i] = value;
    }

    return s;
}

void* memmove(void* dest, const void* src, size_t n) 
{
    uint8_t* pdest = (uint8_t*)dest;
    const uint8_t* psrc = (const uint8_t*)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    }
    else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i - 1] = psrc[i - 1];
        }
    }

    return dest;
}

int memcmp(const void* s1, const void* s2, size_t n)
{
    const uint8_t* p1 = (const uint8_t*)s1;
    const uint8_t* p2 = (const uint8_t*)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

void FillUnique(void* address, uint64_t baseValue, uint64_t byteCount)
{
    uint64_t* ptr = (uint64_t*)address;
    uint64_t numElements = byteCount / 8; // Each uint64_t is 8 bytes

    for (uint64_t i = 0; i < numElements; ++i) {
        ptr[i] = baseValue + i;
    }
}


extern "C" void __chkstk(void)
{

}
