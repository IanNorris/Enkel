#include "kernel/console/console.h"
#include "common/string.h"
#include "kernel/init/gdt.h"
#include "kernel/init/msr.h"
#include "kernel/init/tls.h"
#include "utilities/termination.h"
#include "errno.h"

#include "memory/virtual.h"
#include "memory/memory.h"
#include <rpmalloc.h>

#include "fs/volume.h"

#include "kernel/user_mode/syscall.h"
#include "kernel/process/process.h"

#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/openat2.h>

#include "kernel/memory/pml4.h"
#include "kernel/process/process.h"

#define ARCH_SET_GS 0x1001
#define ARCH_SET_FS 0x1002
#define ARCH_GET_FS 0x1003
#define ARCH_GET_GS 0x1004

#define ARCH_GET_CPUID 0x1011
#define ARCH_SET_CPUID 0x1012

#define ARCH_CET_STATUS 0x3001

extern Process* GCurrentProcess;

extern MemoryState PhysicalMemoryState;

extern EnvironmentKernel* GKernelEnvironment;

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

	//PrintStackTrace(60, rip, rbp);

	return -ENOSYS;
}

int sys_clock_gettime(const clockid_t which_clock, struct timespec* tp)
{
	//TODO
	memset(tp, 0, sizeof(timespec));

	return 0;
}

int sys_prlimit64(pid_t pid, unsigned int resource, const struct rlimit64* new_rlim, struct rlimit64* old_rlim)
{
	return 0;
}

//
uint64_t __attribute__((sysv_abi,used)) sys_arch_prctl(int code, unsigned long *addr)
{
	switch(code)
	{
		case ARCH_SET_FS:
			SetUserFSBase((uint64_t)addr);
			return 0;

		case ARCH_GET_FS:
			return GetUserFSBase();

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

uint64_t sys_access(const char *filename, int mode)
{
	SerialPrint("access: ");
	SerialPrint(filename);
	SerialPrint("\n");

	//TODO check if exists
	//For now, YOLO

	return R_OK;
}

int sys_getrandom(char* buf, size_t count, unsigned int flags)
{
	//TODO - for now we want determinism.

	for (size_t i = 0; i < count; i++)
	{
		buf[i] = i % 256;
	}

	return 0;
}

uint64_t sys_pread64(unsigned long fileHandle, char* data, size_t dataSize, long offset)
{
	return VolumeRead(fileHandle, offset, data, dataSize);
}

int sys_read(uint64_t fileHandle, void* data, uint64_t dataSize)
{
	return VolumeRead(fileHandle, ~0ULL, data, dataSize);
}

int sys_write(uint64_t fileHandle, void* data, uint64_t dataSize)
{
	return VolumeWrite(fileHandle, ~0ULL, data, dataSize);
}

int sys_open(const char* filename, int flags, int mode)
{
	//TODO Buffer check
	char16_t wideName[256];
	ascii_to_wide(wideName, filename, 256);

	uint64_t handle = VolumeOpenHandle(0, wideName, mode);
	if (handle == 0)
	{
		return -ENOENT;
	}
}

int sys_close(uint64_t fileHandle)
{
	VolumeCloseHandle(fileHandle);

	return 0;
}

size_t sys_writev(int fileHandle, const struct iovec* iov, int iovcnt)
{
	size_t total = 0;

	if (iov && iovcnt)
	{
		//TODO NOT THREAD SAFE! This needs to be atomic
		for (int i = 0; i < iovcnt; i++)
		{
			total += VolumeWrite(fileHandle, ~0ULL, iov[i].iov_base, iov[i].iov_len);
		}
	}

	return total;
}

void sys_exit(int64_t exitCode)
{
	ConsolePrint(u"Exit 0x");

	char16_t Buffer[10];
	witoabuf(Buffer, (int32_t)exitCode, 10);
	ConsolePrint(Buffer);
	ConsolePrint(u"\n");

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
		return GCurrentProcess->ProgramBreak;
	}
}

void* sys_memory_map(void *address,size_t length,int prot,int flags,int fd, uint64_t offset)
{
	size_t alignedSize = AlignSize(length, PAGE_SIZE);

	if (address != 0)
	{
		uint8_t* next_address = (uint8_t*)address;
		void* end_address = next_address + alignedSize;

		while (next_address < end_address)
		{
			if (GetPhysicalAddress((uint64_t)next_address) != INVALID_ADDRESS)
			{
				next_address += PAGE_SIZE;
			}
			else
			{
				return (void*)-ENOMEM;
			}
		}

		//All available
		
		//TODO: We don't actually need a contiguous block of physical for this!
		uint64_t PhysicalAddress = PhysicalMemoryState.FindMinimumSizeFreeBlock(alignedSize);

		//TODO
		MapPages((uint64_t)address, PhysicalAddress, alignedSize, /*writable*/true, /*executable*/true, PrivilegeLevel::User, MemoryState::RangeState::Used);
	}

	if (fd == -1)
	{
		void* memory = address != nullptr ? address : VirtualAlloc(alignedSize, PrivilegeLevel::User);
		memset(memory, 0, alignedSize);
		return memory;
	}

	if (fd > 0)
	{
		void* memory = address != nullptr ? address : VirtualAlloc(AlignSize(length, PAGE_SIZE), PrivilegeLevel::User);

		VolumeRead(fd, offset, memory, length);

		return memory;
	}
	else
	{
		return nullptr;
	}
}

int sys_mprotect(unsigned long start, size_t len, unsigned long prot)
{
	//TODO
	return 0;
}

int sys_execve(const char* filename, const char* const argv[], const char* const envp[])
{
	const int64_t maxLength = 1 << 30;

	//Pre-calculate the size of the memory block so we can get args in the right order later ith less fuss
	int envc = 0;
	int argc = 0;
	const char* const* argPointer = envp;

	uint64_t stringBlockBytes = 0;

	stringBlockBytes += ascii_to_wide(nullptr, filename, maxLength) + 1;

	argPointer = envp;
	while (*argPointer)
	{
		stringBlockBytes += ascii_to_wide(nullptr, *argPointer, maxLength) + 1;
		envc++;
		argPointer++;
	}

	argPointer = argv;
	while (*argPointer)
	{
		stringBlockBytes += ascii_to_wide(nullptr, *argPointer, maxLength) + 1;
		argc++;
		argPointer++;
	}

	char16_t** memoryBlockPointers = (char16_t**)rpmalloc((envc + argc + 2) * sizeof(char16_t*));
	char16_t* memoryBlockStrings = (char16_t*)rpmalloc(stringBlockBytes * sizeof(char16_t));
	char16_t* memoryBlockStringsPtr = memoryBlockStrings;
	char16_t** memoryBlockPointersCurrent = (char16_t**)memoryBlockPointers;

	const char16_t* wideFilename = memoryBlockStringsPtr;
	int writing;
	writing = ascii_to_wide(memoryBlockStringsPtr, filename, maxLength);
	memoryBlockStringsPtr[writing] = 0;
	memoryBlockStringsPtr += writing + 1;

	char16_t** wideEnv = memoryBlockPointersCurrent;

	argPointer = envp;
	while (*argPointer)
	{
		*memoryBlockPointersCurrent = memoryBlockStringsPtr;
		writing = ascii_to_wide(memoryBlockStringsPtr, *argPointer, maxLength);
		memoryBlockStringsPtr[writing] = 0;
		memoryBlockStringsPtr += writing + 1;
		memoryBlockPointersCurrent++;
		argPointer++;
	}
	*memoryBlockPointersCurrent = nullptr;
	memoryBlockPointersCurrent++;

	char16_t** wideArg = memoryBlockPointersCurrent;

	argPointer = argv;
	while (*argPointer)
	{
		*memoryBlockPointersCurrent = memoryBlockStringsPtr;
		writing = ascii_to_wide(memoryBlockStringsPtr, *argPointer, maxLength);
		memoryBlockStringsPtr[writing] = 0;
		memoryBlockStringsPtr += writing + 1;
		memoryBlockPointersCurrent++;
		argPointer++;
	}
	*memoryBlockPointersCurrent = nullptr;
	
	int result = RunProgram(wideFilename, (const char16_t**)wideArg, (const char16_t**)wideEnv);

	rpfree(memoryBlockPointers);
	rpfree(memoryBlockStrings);

	return result;
}

extern const char16_t* KernelBuildId;

int sys_uname(struct utsname* buf)
{
	strcpy(buf->sysname, "Enkel");
	strcpy(buf->nodename, "enkel");

	wide_to_ascii(buf->version, KernelBuildId, _UTSNAME_RELEASE_LENGTH);

	// Need a version number newer than what glibc expects
	strcpy(buf->release, "90200");
	

	strcpy(buf->machine, "x86_64");

	return 0;
}

int sys_openat(int dfd, const char* filename, int flags, uint64_t mode)
{
	//TODO Buffer check
	char16_t wideName[256];
	ascii_to_wide(wideName, filename, 256);

	uint64_t handle = VolumeOpenHandle(0, wideName, mode);
	if (handle == 0)
	{
		return -ENOENT;
	}

	return handle;
}

static int inodeTemp = 0;

int sys_newfstatat(int dfd, const char* filename, struct stat* statbuf, int flag)
{
	memset(statbuf, 0, sizeof(statbuf));

	statbuf->st_size = VolumeGetSize(dfd);

	statbuf->st_dev = 0; //TODO
	statbuf->st_ino = ++inodeTemp; //TODO
	statbuf->st_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH; //TODO read only
	statbuf->st_nlink = 0; //TODO
	statbuf->st_uid = 0; //TODO
	statbuf->st_gid = 0; //TODO
	statbuf->st_rdev = 0; //TODO
	
	statbuf->st_blksize = 0; //DO
	statbuf->st_blocks = 0; //TODO
	//statbuf->st_atime = 0; //TODO
	//statbuf->st_mtime = 0; //TODO
	//statbuf->st_ctime = 0; //TODO

	return 0;
}

int sys_set_tid_address(int* tidptr)
{
	*tidptr = 99999; //TODO
	return 99999;
}

int sys_getpid()
{
	//TODO
	return 1;
}

void* SyscallTable[(int)SyscallIndex::Max] = 
{
	(void*)sys_read, // 0,
	(void*)sys_write, // 1,

	(void*)sys_open, // 2,
	(void*)sys_close, // 3,
	(void*)sys_not_implemented, // NotImplemented4,
	(void*)sys_not_implemented, // NotImplemented5,
	(void*)sys_not_implemented, // NotImplemented6,
	(void*)sys_not_implemented, // NotImplemented7,
	(void*)sys_not_implemented, // NotImplemented8,
	(void*)sys_memory_map, 		// MemoryMap,

	(void*)sys_mprotect,		// mprotect
	(void*)sys_not_implemented, // NotImplemented11,
	(void*)sys_brk, // Break,
	(void*)sys_not_implemented, // NotImplemented13,
	(void*)sys_not_implemented, // NotImplemented14,
	(void*)sys_not_implemented, // NotImplemented15,
	(void*)sys_not_implemented, // NotImplemented16,
	(void*)sys_pread64, // 17,
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
	(void*)sys_getpid, // 39,

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
	(void*)sys_execve, // 59,
	
	(void*)sys_exit,			// Exit,
	(void*)sys_not_implemented, // NotImplemented61,
	(void*)sys_not_implemented, // NotImplemented62,
	(void*)sys_uname,			// uname,
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
	(void*)sys_not_implemented, // NotImplemented127,
	(void*)sys_not_implemented, // NotImplemented138,
	(void*)sys_not_implemented, // NotImplemented139,
	(void*)sys_not_implemented, // NotImplemented130,
	(void*)sys_not_implemented, // NotImplemented131,
	(void*)sys_not_implemented, // NotImplemented132,
	(void*)sys_not_implemented, // NotImplemented133,
	(void*)sys_not_implemented, // NotImplemented134,
	(void*)sys_not_implemented, // NotImplemented135,
	(void*)sys_not_implemented, // NotImplemented136,
	(void*)sys_not_implemented, // NotImplemented137,
	(void*)sys_not_implemented, // NotImplemented138,
	(void*)sys_not_implemented, // NotImplemented139,
	(void*)sys_not_implemented, // NotImplemented140,
	(void*)sys_not_implemented, // NotImplemented141,
	(void*)sys_not_implemented, // NotImplemented142,
	(void*)sys_not_implemented, // NotImplemented143,
	(void*)sys_not_implemented, // NotImplemented144,
	(void*)sys_not_implemented, // NotImplemented145,
	(void*)sys_not_implemented, // NotImplemented146,
	(void*)sys_not_implemented, // NotImplemented147,
	(void*)sys_not_implemented, // NotImplemented158,
	(void*)sys_not_implemented, // NotImplemented159,
	(void*)sys_not_implemented, // NotImplemented150,
	(void*)sys_not_implemented, // NotImplemented151,
	(void*)sys_not_implemented, // NotImplemented152,
	(void*)sys_not_implemented, // NotImplemented153,
	(void*)sys_not_implemented, // NotImplemented154,
	(void*)sys_not_implemented, // NotImplemented155,
	(void*)sys_not_implemented, // NotImplemented156,
	(void*)sys_not_implemented, // NotImplemented157,
	(void*)sys_arch_prctl, 		// 158,
	(void*)sys_not_implemented, // NotImplemented159,
	(void*)sys_not_implemented, // NotImplemented160,
	(void*)sys_not_implemented, // NotImplemented161,
	(void*)sys_not_implemented, // NotImplemented162,
	(void*)sys_not_implemented, // NotImplemented163,
	(void*)sys_not_implemented, // NotImplemented164,
	(void*)sys_not_implemented, // NotImplemented165,
	(void*)sys_not_implemented, // NotImplemented166,
	(void*)sys_not_implemented, // NotImplemented167,
	(void*)sys_not_implemented, // NotImplemented168,
	(void*)sys_not_implemented, // NotImplemented169,
	(void*)sys_not_implemented, // NotImplemented170,
	(void*)sys_not_implemented, // NotImplemented171,
	(void*)sys_not_implemented, // NotImplemented172,
	(void*)sys_not_implemented, // NotImplemented173,
	(void*)sys_not_implemented, // NotImplemented174,
	(void*)sys_not_implemented, // NotImplemented175,
	(void*)sys_not_implemented, // NotImplemented176,
	(void*)sys_not_implemented, // NotImplemented177,
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




	(void*)sys_not_implemented, // NotImplemented200,
	(void*)sys_not_implemented, // NotImplemented201,
	(void*)sys_not_implemented, // NotImplemented202,
	(void*)sys_not_implemented, // NotImplemented203,
	(void*)sys_not_implemented, // NotImplemented204,
	(void*)sys_not_implemented, // NotImplemented205,
	(void*)sys_not_implemented, // NotImplemented206,
	(void*)sys_not_implemented, // NotImplemented207,
	(void*)sys_not_implemented, // NotImplemented208,
	(void*)sys_not_implemented, // NotImplemented209,
	(void*)sys_not_implemented, // NotImplemented210,
	(void*)sys_not_implemented, // NotImplemented211,
	(void*)sys_not_implemented, // NotImplemented212,
	(void*)sys_not_implemented, // NotImplemented213,
	(void*)sys_not_implemented, // NotImplemented214,
	(void*)sys_not_implemented, // NotImplemented215,
	(void*)sys_not_implemented, // NotImplemented216,
	(void*)sys_not_implemented, // NotImp1emented217,
	(void*)sys_set_tid_address, // 218,
	(void*)sys_not_implemented, // NotImplemented219,
	(void*)sys_not_implemented, // NotImplemented220,
	(void*)sys_not_implemented, // NotImplemented221,
	(void*)sys_not_implemented, // NotImplemented222,
	(void*)sys_not_implemented, // NotImplemented223,
	(void*)sys_not_implemented, // NotImplemented224,
	(void*)sys_not_implemented, // NotImplemented225,
	(void*)sys_not_implemented, // NotImplemented226,
	(void*)sys_not_implemented, // NotImplemented227,
	(void*)sys_clock_gettime, // 228,
	(void*)sys_not_implemented, // NotImplemented229,
	(void*)sys_not_implemented, // NotImplemented230,
	(void*)sys_exit, // TODO sys_exit_group 231
	(void*)sys_not_implemented, // NotImplemented232,
	(void*)sys_not_implemented, // NotImplemented233,
	(void*)sys_not_implemented, // NotImplemented234,
	(void*)sys_not_implemented, // NotImplemented235,
	(void*)sys_not_implemented, // NotImplemented236,
	(void*)sys_not_implemented, // NotImplemented237,
	(void*)sys_not_implemented, // NotImplemented238,
	(void*)sys_not_implemented, // NotImplemented239,
	(void*)sys_not_implemented, // NotImplemented240,
	(void*)sys_not_implemented, // NotImplemented241,
	(void*)sys_not_implemented, // NotImplemented242,
	(void*)sys_not_implemented, // NotImplemented243,
	(void*)sys_not_implemented, // NotImplemented244,
	(void*)sys_not_implemented, // NotImplemented245,
	(void*)sys_not_implemented, // NotImplemented246,
	(void*)sys_not_implemented, // NotImplemented247,
	(void*)sys_not_implemented, // NotImplemented258,
	(void*)sys_not_implemented, // NotImplemented259,
	(void*)sys_not_implemented, // NotImplemented250,
	(void*)sys_not_implemented, // NotImplemented251,
	(void*)sys_not_implemented, // NotImplemented252,
	(void*)sys_not_implemented, // NotImplemented253,
	(void*)sys_not_implemented, // NotImplemented254,
	(void*)sys_not_implemented, // NotImplemented255,
	(void*)sys_not_implemented, // NotImplemented256,
	(void*)sys_openat, // 257,
	(void*)sys_not_implemented, // NotImplemented258,
	(void*)sys_not_implemented, // NotImplemented259,
	(void*)sys_not_implemented, // NotImplemented260,
	(void*)sys_not_implemented, // NotImplemented261,
	(void*)sys_newfstatat, // 262,
	(void*)sys_not_implemented, // NotImplemented263,
	(void*)sys_not_implemented, // NotImplemented264,
	(void*)sys_not_implemented, // NotImplemented265,
	(void*)sys_not_implemented, // NotImplemented266,
	(void*)sys_not_implemented, // NotImplemented267,
	(void*)sys_not_implemented, // NotImplemented268,
	(void*)sys_not_implemented, // NotImplemented269,
	(void*)sys_not_implemented, // NotImplemented270,
	(void*)sys_not_implemented, // NotImplemented271,
	(void*)sys_not_implemented, // NotImplemented272,
	(void*)sys_not_implemented, // NotImplemented273,
	(void*)sys_not_implemented, // NotImplemented274,
	(void*)sys_not_implemented, // NotImplemented275,
	(void*)sys_not_implemented, // NotImplemented276,
	(void*)sys_not_implemented, // NotImplemented277,
	(void*)sys_not_implemented, // NotImplemented278,
	(void*)sys_not_implemented, // NotImplemented279,
	(void*)sys_not_implemented, // NotImplemented280,
	(void*)sys_not_implemented, // NotImplemented281,
	(void*)sys_not_implemented, // NotImplemented282,
	(void*)sys_not_implemented, // NotImplemented283,
	(void*)sys_not_implemented, // NotImplemented284,
	(void*)sys_not_implemented, // NotImplemented285,
	(void*)sys_not_implemented, // NotImplemented286,
	(void*)sys_not_implemented, // NotImplemented287,
	(void*)sys_not_implemented, // NotImplemented288,
	(void*)sys_not_implemented, // NotImplemented289,
	(void*)sys_not_implemented, // NotImplemented290,
	(void*)sys_not_implemented, // NotImplemented291,
	(void*)sys_not_implemented, // NotImplemented292,
	(void*)sys_not_implemented, // NotImplemented293,
	(void*)sys_not_implemented, // NotImplemented294,
	(void*)sys_not_implemented, // NotImplemented295,
	(void*)sys_not_implemented, // NotImplemented296,
	(void*)sys_not_implemented, // NotImp9emented297,
	(void*)sys_not_implemented, // NotImplemented298,
	(void*)sys_not_implemented, // NotImplemented299,


	(void*)sys_not_implemented, // NotImplemented300,
	(void*)sys_not_implemented, // NotImplemented301,
	(void*)sys_prlimit64, // 302,
	(void*)sys_not_implemented, // NotImplemented303,
	(void*)sys_not_implemented, // NotImplemented304,
	(void*)sys_not_implemented, // NotImplemented305,
	(void*)sys_not_implemented, // NotImplemented306,
	(void*)sys_not_implemented, // NotImplemented307,
	(void*)sys_not_implemented, // NotImplemented308,
	(void*)sys_not_implemented, // NotImplemented309,
	(void*)sys_not_implemented, // NotImplemented310,
	(void*)sys_not_implemented, // NotImplemented311,
	(void*)sys_not_implemented, // NotImplemented312,
	(void*)sys_not_implemented, // NotImplemented313,
	(void*)sys_not_implemented, // NotImplemented314,
	(void*)sys_not_implemented, // NotImplemented315,
	(void*)sys_not_implemented, // NotImplemented316,
	(void*)sys_not_implemented, // NotImp1emented317,
	(void*)sys_getrandom, // 318,
	(void*)sys_not_implemented, // NotImplemented319,
	(void*)sys_not_implemented, // NotImplemented320,
	(void*)sys_not_implemented, // NotImplemented321,
	(void*)sys_not_implemented, // NotImplemented322,
	(void*)sys_not_implemented, // NotImplemented323,
	(void*)sys_not_implemented, // NotImplemented324,
	(void*)sys_not_implemented, // NotImplemented325,
	(void*)sys_not_implemented, // NotImplemented326,
	(void*)sys_not_implemented, // NotImplemented327,
	(void*)sys_not_implemented, // NotImplemented338,
	(void*)sys_not_implemented, // NotImplemented339,
	(void*)sys_not_implemented, // NotImplemented330,
	(void*)sys_not_implemented, // NotImplemented331,
	(void*)sys_not_implemented, // NotImplemented332,
	(void*)sys_not_implemented, // NotImplemented333,
	(void*)sys_not_implemented, // NotImplemented334,
	(void*)sys_not_implemented, // NotImplemented335,
	(void*)sys_not_implemented, // NotImplemented336,
	(void*)sys_not_implemented, // NotImplemented337,
	(void*)sys_not_implemented, // NotImplemented338,
	(void*)sys_not_implemented, // NotImplemented339,
	(void*)sys_not_implemented, // NotImplemented340,
	(void*)sys_not_implemented, // NotImplemented341,
	(void*)sys_not_implemented, // NotImplemented342,
	(void*)sys_not_implemented, // NotImplemented343,
	(void*)sys_not_implemented, // NotImplemented344,
	(void*)sys_not_implemented, // NotImplemented345,
	(void*)sys_not_implemented, // NotImplemented346,
	(void*)sys_not_implemented, // NotImplemented347,
	(void*)sys_not_implemented, // NotImplemented358,
	(void*)sys_not_implemented, // NotImplemented359,
	(void*)sys_not_implemented, // NotImplemented350,
	(void*)sys_not_implemented, // NotImplemented351,
	(void*)sys_not_implemented, // NotImplemented352,
	(void*)sys_not_implemented, // NotImplemented353,
	(void*)sys_not_implemented, // NotImplemented354,
	(void*)sys_not_implemented, // NotImplemented355,
	(void*)sys_not_implemented, // NotImplemented356,
	(void*)sys_not_implemented, // NotImplemented357,
	(void*)sys_not_implemented, // NotImplemented358,
	(void*)sys_not_implemented, // NotImplemented359,
	(void*)sys_not_implemented, // NotImplemented360,
	(void*)sys_not_implemented, // NotImplemented361,
	(void*)sys_not_implemented, // NotImplemented362,
	(void*)sys_not_implemented, // NotImplemented363,
	(void*)sys_not_implemented, // NotImplemented364,
	(void*)sys_not_implemented, // NotImplemented365,
	(void*)sys_not_implemented, // NotImplemented366,
	(void*)sys_not_implemented, // NotImplemented367,
	(void*)sys_not_implemented, // NotImplemented368,
	(void*)sys_not_implemented, // NotImplemented369,
	(void*)sys_not_implemented, // NotImplemented370,
	(void*)sys_not_implemented, // NotImplemented371,
	(void*)sys_not_implemented, // NotImplemented372,
	(void*)sys_not_implemented, // NotImplemented373,
	(void*)sys_not_implemented, // NotImplemented374,
	(void*)sys_not_implemented, // NotImplemented375,
	(void*)sys_not_implemented, // NotImplemented376,
	(void*)sys_not_implemented, // NotImplemented377,
	(void*)sys_not_implemented, // NotImplemented378,
	(void*)sys_not_implemented, // NotImplemented379,
	(void*)sys_not_implemented, // NotImplemented380,
	(void*)sys_not_implemented, // NotImplemented381,
	(void*)sys_not_implemented, // NotImplemented382,
	(void*)sys_not_implemented, // NotImplemented383,
	(void*)sys_not_implemented, // NotImplemented384,
	(void*)sys_not_implemented, // NotImplemented385,
	(void*)sys_not_implemented, // NotImplemented386,
	(void*)sys_not_implemented, // NotImplemented387,
	(void*)sys_not_implemented, // NotImplemented388,
	(void*)sys_not_implemented, // NotImplemented389,
	(void*)sys_not_implemented, // NotImplemented390,
	(void*)sys_not_implemented, // NotImplemented391,
	(void*)sys_not_implemented, // NotImplemented392,
	(void*)sys_not_implemented, // NotImplemented393,
	(void*)sys_not_implemented, // NotImplemented394,
	(void*)sys_not_implemented, // NotImplemented395,
	(void*)sys_not_implemented, // NotImplemented396,
	(void*)sys_not_implemented, // NotImp9emented397,
	(void*)sys_not_implemented, // NotImplemented398,
	(void*)sys_not_implemented, // NotImplemented399,
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

	GKernelEnvironment->SyscallStack = (uint64_t)VirtualAlloc(16 * 1024, PrivilegeLevel::Kernel) + (16 * 1024);
	GKernelEnvironment->SyscallTable = (uint64_t)SyscallTable;
}