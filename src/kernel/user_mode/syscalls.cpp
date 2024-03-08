#include "kernel/console/console.h"
#include "common/string.h"
#include "kernel/init/gdt.h"
#include "kernel/init/msr.h"
#include "kernel/init/tls.h"
#include "utilities/termination.h"
#include "errno.h"

#include "memory/virtual.h"

#include "fs/volume.h"

#include "kernel/user_mode/syscall.h"
#include "kernel/process/process.h"

#define ARCH_SET_GS 0x1001
#define ARCH_SET_FS 0x1002
#define ARCH_GET_FS 0x1003
#define ARCH_GET_GS 0x1004

#define ARCH_GET_CPUID 0x1011
#define ARCH_SET_CPUID 0x1012

#define ARCH_CET_STATUS 0x3001

extern Process* GCurrentProcess;

extern "C"
{
	uint64_t SyscallStack;
}

extern "C" void __attribute__((sysv_abi,noreturn)) ReturnToKernel();

extern "C" uint64_t sys_not_implemented()
{
	unsigned long syscall_number;
	uint64_t rip;
	uint64_t rsp;
	uint64_t rbp;
	asm ("movq %%rax, %0" : "=r" (syscall_number));
	asm ("movq %%rcx, %0" : "=r" (rip));
	asm ("movq %%r13, %0" : "=r" (rsp));
	asm ("movq %%rbx, %0" : "=r" (rbp));

	ConsolePrint(u"Syscall ");

	char16_t Buffer[10];
	witoabuf(Buffer, syscall_number, 10);
	ConsolePrint(Buffer);

	ConsolePrint(u" not implemented\n");

	ConsolePrint(u"RIP: 0x");

	witoabuf(Buffer, rip, 16, 8);
	ConsolePrint(Buffer);

	ConsolePrint(u"\n");

	PrintStackTrace(60, rip, rbp);

	return EINVAL;
}

//
uint64_t __attribute__((sysv_abi,used)) sys_arch_prctl(int code, unsigned long *addr)
{
	switch(code)
	{
		case ARCH_SET_FS:
			SetFSBase((uint64_t)addr);
			return 0;

		case ARCH_GET_FS:
			return GetFSBase();

		case ARCH_CET_STATUS:
			return -EINVAL;

		default:
			{
				ConsolePrint(u"sys_arch_prctl code 0x");

				char16_t Buffer[10];
				witoabuf(Buffer, code, 16);
				ConsolePrint(Buffer);

				ConsolePrint(u" not implemented\n");				
			}
			//NOT_IMPLEMENTED();
			return -EINVAL;
	}
}

uint64_t sys_access(const char *pathname, int mode)
{
	SerialPrint(pathname);
	SerialPrint("\n");

	return -1;
}

void sys_write(uint64_t fileHandle, void* data, uint64_t dataSize)
{
	VolumeWrite(fileHandle, ~0ULL, data, dataSize);
}

struct iovec
{
	void* iov_base;  /* Starting address of the buffer */
	size_t iov_len;   /* Length of the buffer */
};

size_t sys_writev(int fileHandle, const struct iovec* iov, int iovcnt)
{
	size_t total = 0;

	//TODO NOT THREAD SAFE! This needs to be atomic
	for (int i = 0; i < iovcnt; i++)
	{
		total += VolumeWrite(fileHandle, ~0ULL, iov[i].iov_base, iov[i].iov_len);
	}

	return total;
}

void sys_exit(int64_t exitCode)
{
	/*ConsolePrint(u"Exit 0x");

	char16_t Buffer[10];
	witoabuf(Buffer, (int32_t)exitCode, 10);
	ConsolePrint(Buffer);
	ConsolePrint(u"\n");*/

	ReturnToKernel();
}

uint64_t sys_brk(uint64_t newBreakAddress)
{
	if(newBreakAddress > GCurrentProcess->Binary->ProgramBreakLow)
	{
		if(newBreakAddress <= GCurrentProcess->Binary->ProgramBreakHigh)
		{
			GCurrentProcess->ProgramBreak = newBreakAddress;
			return newBreakAddress;
		}
		else
		{
			_ASSERTF(false, "Exceeded allocated program break");
		}
	}
	else
	{
		return GCurrentProcess->Binary->ProgramBreakLow;
	}
}

void* sys_memory_map(void *address,size_t length,int prot,int flags,int fd, uint64_t offset)
{
	return VirtualAlloc(AlignSize(length, PAGE_SIZE),  PrivilegeLevel::User);
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
	(void*)sys_memory_map, 		// MemoryMap,

	(void*)sys_not_implemented, // NotImplemented10,
	(void*)sys_not_implemented, // NotImplemented11,
	(void*)sys_brk, // Break,
	(void*)sys_not_implemented, // NotImplemented13,
	(void*)sys_not_implemented, // NotImplemented14,
	(void*)sys_not_implemented, // NotImplemented15,
	(void*)sys_not_implemented, // NotImplemented16,
	(void*)sys_not_implemented, // NotImplemented17,
	(void*)sys_not_implemented, // NotImplemented18,
	(void*)sys_not_implemented, // NotImplemented19,

	(void*)sys_writev,			// 20,
	(void*)sys_access, 			// 21,
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
	(void*)sys_not_implemented, // NotImplemented61,
	(void*)sys_not_implemented, // NotImplemented62,
	(void*)sys_not_implemented, // NotImplemented63,
	(void*)sys_not_implemented, // NotImplemented64,
	(void*)sys_not_implemented, // NotImplemented65,
	(void*)sys_not_implemented, // NotImplemented66,
	(void*)sys_not_implemented, // NotImplemented67,
	(void*)sys_not_implemented, // NotImplemented68,
	(void*)sys_not_implemented, // NotImplemented69,

	(void*)sys_not_implemented, // NotImplemented70,
	(void*)sys_not_implemented, // NotImplemented71,
	(void*)sys_not_implemented, // NotImplemented72,
	(void*)sys_not_implemented, // NotImplemented73,
	(void*)sys_not_implemented, // NotImplemented74,
	(void*)sys_not_implemented, // NotImplemented75,
	(void*)sys_not_implemented, // NotImplemented76,
	(void*)sys_not_implemented, // NotImplemented77,
	(void*)sys_not_implemented, // NotImplemented78,
	(void*)sys_not_implemented, // NotImplemented79,

	(void*)sys_not_implemented, // NotImplemented80,
	(void*)sys_not_implemented, // NotImplemented81,
	(void*)sys_not_implemented, // NotImplemented82,
	(void*)sys_not_implemented, // NotImplemented83,
	(void*)sys_not_implemented, // NotImplemented84,
	(void*)sys_not_implemented, // NotImplemented85,
	(void*)sys_not_implemented, // NotImplemented86,
	(void*)sys_not_implemented, // NotImplemented87,
	(void*)sys_not_implemented, // NotImplemented88,
	(void*)sys_not_implemented, // NotImplemented89,

	(void*)sys_not_implemented, // NotImplemented90,
	(void*)sys_not_implemented, // NotImplemented91,
	(void*)sys_not_implemented, // NotImplemented92,
	(void*)sys_not_implemented, // NotImplemented93,
	(void*)sys_not_implemented, // NotImplemented94,
	(void*)sys_not_implemented, // NotImplemented95,
	(void*)sys_not_implemented, // NotImplemented96,
	(void*)sys_not_implemented, // NotImplemented97,
	(void*)sys_not_implemented, // NotImplemented98,
	(void*)sys_not_implemented, // NotImplemented99,

	(void*)sys_not_implemented, // NotImplemented100,
	(void*)sys_not_implemented, // NotImplemented101,
	(void*)sys_not_implemented, // NotImplemented102,
	(void*)sys_not_implemented, // NotImplemented103,
	(void*)sys_not_implemented, // NotImplemented104,
	(void*)sys_not_implemented, // NotImplemented105,
	(void*)sys_not_implemented, // NotImplemented106,
	(void*)sys_not_implemented, // NotImplemented107,
	(void*)sys_not_implemented, // NotImplemented108,
	(void*)sys_not_implemented, // NotImplemented109,
	(void*)sys_not_implemented, // NotImplemented110,
	(void*)sys_not_implemented, // NotImplemented111,
	(void*)sys_not_implemented, // NotImplemented112,
	(void*)sys_not_implemented, // NotImplemented113,
	(void*)sys_not_implemented, // NotImplemented114,
	(void*)sys_not_implemented, // NotImplemented115,
	(void*)sys_not_implemented, // NotImplemented116,
	(void*)sys_not_implemented, // NotImp1emented117,
	(void*)sys_not_implemented, // NotImplemented118,
	(void*)sys_not_implemented, // NotImplemented119,
	(void*)sys_not_implemented, // NotImplemented120,
	(void*)sys_not_implemented, // NotImplemented121,
	(void*)sys_not_implemented, // NotImplemented122,
	(void*)sys_not_implemented, // NotImplemented123,
	(void*)sys_not_implemented, // NotImplemented124,
	(void*)sys_not_implemented, // NotImplemented125,
	(void*)sys_not_implemented, // NotImplemented126,
	(void*)sys_not_implemented, // NotImp2emented127,
	(void*)sys_not_implemented, // NotImplemented138,
	(void*)sys_not_implemented, // NotImplemented139,
	(void*)sys_not_implemented, // NotImplemented130,
	(void*)sys_not_implemented, // NotImplemented131,
	(void*)sys_not_implemented, // NotImplemented132,
	(void*)sys_not_implemented, // NotImplemented133,
	(void*)sys_not_implemented, // NotImplemented134,
	(void*)sys_not_implemented, // NotImplemented135,
	(void*)sys_not_implemented, // NotImplemented136,
	(void*)sys_not_implemented, // NotImp3emented137,
	(void*)sys_not_implemented, // NotImplemented138,
	(void*)sys_not_implemented, // NotImplemented139,
	(void*)sys_not_implemented, // NotImplemented140,
	(void*)sys_not_implemented, // NotImplemented141,
	(void*)sys_not_implemented, // NotImplemented142,
	(void*)sys_not_implemented, // NotImplemented143,
	(void*)sys_not_implemented, // NotImplemented144,
	(void*)sys_not_implemented, // NotImplemented145,
	(void*)sys_not_implemented, // NotImplemented146,
	(void*)sys_not_implemented, // NotImp4emented147,
	(void*)sys_not_implemented, // NotImplemented158,
	(void*)sys_not_implemented, // NotImplemented159,
	(void*)sys_not_implemented, // NotImplemented150,
	(void*)sys_not_implemented, // NotImplemented151,
	(void*)sys_not_implemented, // NotImplemented152,
	(void*)sys_not_implemented, // NotImplemented153,
	(void*)sys_not_implemented, // NotImplemented154,
	(void*)sys_not_implemented, // NotImplemented155,
	(void*)sys_not_implemented, // NotImplemented156,
	(void*)sys_not_implemented, // NotImp5emented157,
	(void*)sys_arch_prctl, 		// 158,
	(void*)sys_not_implemented, // NotImplemented159,
	(void*)sys_not_implemented, // NotImplemented160,
	(void*)sys_not_implemented, // NotImplemented161,
	(void*)sys_not_implemented, // NotImplemented162,
	(void*)sys_not_implemented, // NotImplemented163,
	(void*)sys_not_implemented, // NotImplemented164,
	(void*)sys_not_implemented, // NotImplemented165,
	(void*)sys_not_implemented, // NotImplemented166,
	(void*)sys_not_implemented, // NotImp6emented167,
	(void*)sys_not_implemented, // NotImplemented168,
	(void*)sys_not_implemented, // NotImplemented169,
	(void*)sys_not_implemented, // NotImplemented170,
	(void*)sys_not_implemented, // NotImplemented171,
	(void*)sys_not_implemented, // NotImplemented172,
	(void*)sys_not_implemented, // NotImplemented173,
	(void*)sys_not_implemented, // NotImplemented174,
	(void*)sys_not_implemented, // NotImplemented175,
	(void*)sys_not_implemented, // NotImplemented176,
	(void*)sys_not_implemented, // NotImp7emented177,
	(void*)sys_not_implemented, // NotImplemented178,
	(void*)sys_not_implemented, // NotImplemented179,
	(void*)sys_not_implemented, // NotImplemented180,
	(void*)sys_not_implemented, // NotImplemented181,
	(void*)sys_not_implemented, // NotImplemented182,
	(void*)sys_not_implemented, // NotImplemented183,
	(void*)sys_not_implemented, // NotImplemented184,
	(void*)sys_not_implemented, // NotImplemented185,
	(void*)sys_not_implemented, // NotImplemented186,
	(void*)sys_not_implemented, // NotImp8emented187,
	(void*)sys_not_implemented, // NotImplemented188,
	(void*)sys_not_implemented, // NotImplemented189,
	(void*)sys_not_implemented, // NotImplemented190,
	(void*)sys_not_implemented, // NotImplemented191,
	(void*)sys_not_implemented, // NotImplemented192,
	(void*)sys_not_implemented, // NotImplemented193,
	(void*)sys_not_implemented, // NotImplemented194,
	(void*)sys_not_implemented, // NotImplemented195,
	(void*)sys_not_implemented, // NotImplemented196,
	(void*)sys_not_implemented, // NotImp9emented197,
	(void*)sys_not_implemented, // NotImplemented198,
	(void*)sys_not_implemented, // NotImplemented199,
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

	SyscallStack = (uint64_t)VirtualAlloc(16 * 1024, PrivilegeLevel::Kernel) + (16 * 1024);
}