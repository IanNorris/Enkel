#include <cstddef>
#include <cstdint>
#include <string.h>

size_t _strlen(const char* str)
{
	const char* s = str;
	while (*s)
		++s;
	return s - str;
}

extern "C" size_t __attribute__((sysv_abi)) Write(int fd, const void *buf, size_t count);
extern "C" size_t __attribute__((sysv_abi)) InvSysCall();
extern "C" void __attribute__((sysv_abi,noreturn)) Exit(int errorCode);

void PrintString(const char* message)
{
	Write(1, message, _strlen(message));
}

void PrintError(const char* message)
{
	Write(2, message, _strlen(message));
}

extern "C"
{
void __attribute__((sysv_abi,noreturn,noinline)) _start()
{
	PrintString("Hello World!\n");
	PrintError("I. AM. ERROR.\n");

	PrintString("Invalid syscall test.\n");
	InvSysCall();

	Exit(0);
}
}
