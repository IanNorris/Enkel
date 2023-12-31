#include <cstddef>
#include <cstdint>
#include <string.h>

extern void (*__preinit_array_start []) (void) __attribute__((weak));
extern void (*__preinit_array_end []) (void) __attribute__((weak));
extern void (*__init_array_start []) (void) __attribute__((weak));
extern void (*__init_array_end []) (void) __attribute__((weak));
extern void (*__fini_array_start []) (void) __attribute__((weak));
extern void (*__fini_array_end []) (void) __attribute__((weak));

static void _init()
{

}

static void _fini()
{
	
}

static void __libc_init_array() {
    size_t count, i;
    
    count = __preinit_array_end - __preinit_array_start;
    for (i = 0; i < count; i++)
        __preinit_array_start[i]();
    
    _init();

    count = __init_array_end - __init_array_start;
    for (i = 0; i < count; i++)
        __init_array_start[i]();
}

static void __libc_fini_array() {
    size_t count, i;
    
    count = __preinit_array_end - __preinit_array_start;
    for (i = count - 1; i >= 0; i--)
        __fini_array_start[i]();
    
    _fini();
}


size_t _strlen(const char* str)
{
	const char* s = str;
	while (*s)
		++s;
	return s - str;
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

	while(padDigits > offset)
	{
		buffer[offset++] = '0';
	}

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

extern "C" size_t __attribute__((sysv_abi)) Write(int fd, const void *buf, size_t count);
extern "C" void __attribute__((sysv_abi,noreturn)) Exit(int errorCode);

void PrintString(const char* message)
{
	Write(1, message, _strlen(message));
}

char SpecialMessage[] = "TLS test suite... ";

volatile thread_local uint64_t TLSThing0 = 0xaaaaaaaaaaaaaaaa;
volatile thread_local uint64_t TLSThing1 = 0xbbbbbbbbbbbbbbbb;
volatile thread_local uint64_t TLSThing2 = 0xcccccccccccccccc;
volatile thread_local uint64_t TLSThing3 = 0xdddddddddddddddd;
volatile thread_local uint64_t TLSThing4 = 0xeeeeeeeeeeeeeeee;
volatile thread_local uint64_t TLSThing5 = 0xffffffffffffffff;
volatile thread_local uint64_t TLSThing6 = 0x0;

uint64_t read_fs_offset(uint64_t offset)
{
    uint64_t value;
    __asm__("movq %%fs:(%1), %0"  // Use movq for 64-bit
            : "=r" (value)        // Output operand
            : "r" (offset)        // Input operand
            :);                   // Clobbered register list
    return value;
}

int GetMyStatic()
{
	static int Thing = 1;

	return Thing++;
}

int GetMyStaticTBSS()
{
	static int Thingy;
	Thingy = 0;

	return Thingy++;
}

extern "C"
{
void __attribute__((sysv_abi,noreturn,noinline)) _start()
{
	__libc_init_array();

	PrintString(SpecialMessage);

	for(int i = 0; i < 3; ++i)
	{
		GetMyStatic();
	}

	if(GetMyStatic() != 4)
	{
		PrintString("Static data mismatch!\n");
	}

	thread_local uint64_t tlsTBSS;
	tlsTBSS = TLSThing2;

	if(tlsTBSS != 0xcccccccccccccccc)
	{
		PrintString("TLS test data 1 not aligned!\n");
		Exit(1);
	}

	if(TLSThing4 != 0xeeeeeeeeeeeeeeee)
	{
		PrintString("TLS test data 2 not aligned!\n");
		Exit(2);
	}

	if(TLSThing3 != 0xdddddddddddddddd)
	{
		PrintString("TLS test data 3 not aligned!\n");
		Exit(3);
	}

	TLSThing6 = 0x1234567890abcdef;

	if(TLSThing6 != 0x1234567890abcdef)
	{
		PrintString("TLS test data 4 not written back!\n");
		Exit(4);
	}

	PrintString("OK\n");

	Exit(0);
}
}
