#include <cstddef>
#include <cstdint>
#include <string.h>

// The syscall number for 'write' on x86-64 Linux.
constexpr uint64_t SyscallWrite = 1;

size_t _strlen(const char* str)
{
	const char* s = str;
	while (*s)
		++s;
	return s - str;
}

size_t Write(int fd, const void *buf, size_t count)
{
    size_t result;

    __asm__ volatile (
        "syscall"
        : "=a" (result)                               // The result will be in the rax register
        : "a" (SyscallWrite),                         // The syscall number for 'write'
          "D" ((uint64_t)fd),                         // 'fd' goes in rdi
          "S" (buf),                                  // 'buf' goes in rsi
          "d" (count)                                 // 'count' goes in rdx
        : "rcx", "r11", "memory", "cc"                // rcx and r11 are clobbered by syscall
    );

    return result;
}

void PrintString(const char* message)
{
	Write(1, message, _strlen(message));
}

extern "C"
{
void _start()
{
	PrintString("Hello World!\n");
}
}
