#include "kernel/console/console.h"
#include "common/string.h"
#include "kernel/init/gdt.h"
#include "kernel/init/msr.h"

#include "kernel/user_mode/syscall.h"

void __attribute__((sysv_abi,used)) sys_not_implemented()
{
	_ASSERTF(false, "Syscall not implemented");
}

void sys_write(uint64_t fileHandle, void* data, uint64_t dataSize)
{
	char* stringPtr = (char*)data;

	for(uint64_t i = 0; i < dataSize; i++)
	{
		char16_t buffer[2] = {0,0};
		buffer[0] = (char16_t)stringPtr[i];

		ConsolePrint(buffer);
	}
}

extern "C" void __attribute__((sysv_abi,noreturn)) ReturnToKernel();

void sys_exit(int64_t exitCode)
{
	/*ConsolePrint(u"Exit 0x");

	char16_t Buffer[10];
	witoabuf(Buffer, (int32_t)exitCode, 10);
	ConsolePrint(Buffer);
	ConsolePrint(u"\n");*/

	ReturnToKernel();
}

void* SyscallTable[(int)SyscallIndex::Max] = 
{
	(void*)sys_not_implemented, // Read,
	(void*)sys_write, // Write,

	(void*)sys_not_implemented, // NotImplemented2,
	(void*)sys_not_implemented, // NotImplemented3,
	(void*)sys_not_implemented, // NotImplemented4,
	(void*)sys_not_implemented, // NotImplemented5,
	(void*)sys_not_implemented, // NotImplemented6,
	(void*)sys_not_implemented, // NotImplemented7,
	(void*)sys_not_implemented, // NotImplemented8,
	(void*)sys_not_implemented, // NotImplemented9,

	(void*)sys_not_implemented, // NotImplemented10,
	(void*)sys_not_implemented, // NotImplemented11,
	(void*)sys_not_implemented, // NotImplemented12,
	(void*)sys_not_implemented, // NotImplemented13,
	(void*)sys_not_implemented, // NotImplemented14,
	(void*)sys_not_implemented, // NotImplemented15,
	(void*)sys_not_implemented, // NotImplemented16,
	(void*)sys_not_implemented, // NotImplemented17,
	(void*)sys_not_implemented, // NotImplemented18,
	(void*)sys_not_implemented, // NotImplemented19,

	(void*)sys_not_implemented, // NotImplemented20,
	(void*)sys_not_implemented, // NotImplemented21,
	(void*)sys_not_implemented, // NotImplemented22,
	(void*)sys_not_implemented, // NotImplemented23,
	(void*)sys_not_implemented, // NotImplemented24,
	(void*)sys_not_implemented, // NotImplemented25,
	(void*)sys_not_implemented, // NotImplemented26,
	(void*)sys_not_implemented, // NotImplemented27,
	(void*)sys_not_implemented, // NotImplemented28,
	(void*)sys_not_implemented, // NotImplemented29,

	(void*)sys_not_implemented, // NotImplemented30,
	(void*)sys_not_implemented, // NotImplemented31,
	(void*)sys_not_implemented, // NotImplemented32,
	(void*)sys_not_implemented, // NotImplemented33,
	(void*)sys_not_implemented, // NotImplemented34,
	(void*)sys_not_implemented, // NotImplemented35,
	(void*)sys_not_implemented, // NotImplemented36,
	(void*)sys_not_implemented, // NotImplemented37,
	(void*)sys_not_implemented, // NotImplemented38,
	(void*)sys_not_implemented, // NotImplemented39,

	(void*)sys_not_implemented, // NotImplemented40,
	(void*)sys_not_implemented, // NotImplemented41,
	(void*)sys_not_implemented, // NotImplemented42,
	(void*)sys_not_implemented, // NotImplemented43,
	(void*)sys_not_implemented, // NotImplemented44,
	(void*)sys_not_implemented, // NotImplemented45,
	(void*)sys_not_implemented, // NotImplemented46,
	(void*)sys_not_implemented, // NotImplemented47,
	(void*)sys_not_implemented, // NotImplemented48,
	(void*)sys_not_implemented, // NotImplemented49,

	(void*)sys_not_implemented, // NotImplemented50,
	(void*)sys_not_implemented, // NotImplemented51,
	(void*)sys_not_implemented, // NotImplemented52,
	(void*)sys_not_implemented, // NotImplemented53,
	(void*)sys_not_implemented, // NotImplemented54,
	(void*)sys_not_implemented, // NotImplemented55,
	(void*)sys_not_implemented, // NotImplemented56,
	(void*)sys_not_implemented, // NotImplemented57,
	(void*)sys_not_implemented, // NotImplemented58,
	(void*)sys_not_implemented, // NotImplemented59,
	
	(void*)sys_exit, // Exit,
};


extern "C" uint64_t SyscallTablePointer;
extern "C" uint64_t SyscallDispatcher;

// Define the MSR constants
constexpr uint32_t IA32_STAR = 0xC0000081;
constexpr uint32_t IA32_LSTAR = 0xC0000082;
constexpr uint32_t IA32_FMASK = 0xC0000084;
constexpr uint32_t IA32_EFER = 0xC0000080;
constexpr uint64_t EFER_SCE = 0x0001;

void InitializeSyscalls()
{
    // Set system call entry point
    SetMSR(IA32_LSTAR, (uint64_t)&SyscallDispatcher);

	uint16_t kernelCS = sizeof(GDTEntry) * (uint16_t)GDTEntryIndex::KernelCode;
	uint16_t userDS = sizeof(GDTEntry) * (uint16_t)GDTEntryIndex::UserData;

    uint64_t kernelCodeSegmentShifted = static_cast<uint64_t>(kernelCS) << 3;
	uint64_t userDataSegmentShifted = static_cast<uint64_t>(userDS) << 3;

	// Now, shift them into the correct bit positions for the IA32_STAR MSR
	uint64_t starMsrValue = (userDataSegmentShifted << 48) | (kernelCodeSegmentShifted << 32);

	SetMSR(IA32_STAR, starMsrValue);

    // Clear EFLAGS during syscall
    SetMSR(IA32_FMASK, 0x00000200);

    // Enable SYSCALL/SYSRET
    uint64_t efer = GetMSR(IA32_EFER);
    efer |= EFER_SCE;
    SetMSR(IA32_EFER, efer);
}