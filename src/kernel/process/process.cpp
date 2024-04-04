#include "memory/memory.h"
#include "utilities/termination.h"
#include "kernel/init/gdt.h"
#include "kernel/init/tls.h"
#include "kernel/memory/state.h"
#include "memory/virtual.h"
#include "common/string.h"
#include <rpmalloc.h>
#include "kernel/user_mode/elf.h"
#include "elf.h"

extern EnvironmentKernel* GKernelEnvironment;

int GELFBinaryCount = 0;
ElfBinary** GELFBinaries = nullptr;

Process* GCurrentProcess = nullptr;

void InitializeUserMode()
{
	GELFBinaries = (ElfBinary**)rpmalloc(sizeof(ElfBinary*) * 16);
	GELFBinaryCount = 0;
}

//TODO: Digest https://gist.github.com/x0nu11byt3/bcb35c3de461e5fb66173071a2379779

extern "C" void SwitchToUserMode(uint64_t stackPointer, uint64_t entry, uint16_t userModeCS, uint16_t userModeDS);

void ScheduleProcess(Process* process)
{
	const uint16_t userModeCodeSelector = ((uint16_t)GDTEntryIndex::UserCode * sizeof(GDTEntry)) | 0x3; // Selector | Ring 3
	const uint16_t userModeDataSelector = ((uint16_t)GDTEntryIndex::UserData * sizeof(GDTEntry)) | 0x3;

	GCurrentProcess = process;

	SetUserFSBase((uint64_t)process->TLS->FSBase);
	SetUserGS();
	
	SwitchToUserMode((uint64_t)process->DefaultThreadStackStart, process->Binary->Entry, userModeCodeSelector, userModeDataSelector);

	ReloadSegments();
	SetKernelGSBase((uint64_t)GKernelEnvironment);
	SetFSBase(GKernelEnvironment->FSBase);

	GCurrentProcess = nullptr;
}

uint64_t* WriteAuxEntry(uint64_t* stackPointer, uint64_t auxEntry, uint64_t auxValue, bool dryRun)
{
	stackPointer -= 2;

	if (!dryRun)
	{
		*(stackPointer) = auxEntry;
		*(stackPointer+1) = auxValue;
	}

	return stackPointer;
}

uint64_t* WriteProcessAuxVectors(Process* process, uint64_t* stackPointer, bool dryRun, uint64_t* stackCookie)
{
	stackPointer = WriteAuxEntry(stackPointer, AT_NULL, 0, dryRun);
	stackPointer = WriteAuxEntry(stackPointer, AT_PAGESZ, 4096, dryRun);

	stackPointer = WriteAuxEntry(stackPointer, AT_RANDOM, (uint64_t)stackCookie, dryRun); //TODO make actually random

	if (process->SubBinary)
	{
		stackPointer = WriteAuxEntry(stackPointer, AT_ENTRY, process->SubBinary->Entry, dryRun);
		stackPointer = WriteAuxEntry(stackPointer, AT_BASE, process->SubBinary->BaseAddress, dryRun);
		stackPointer = WriteAuxEntry(stackPointer, AT_PHNUM, process->SubBinary->ProgramHeaderCount, dryRun);
		stackPointer = WriteAuxEntry(stackPointer, AT_PHENT, process->SubBinary->ProgramHeaderEntrySize, dryRun);
		stackPointer = WriteAuxEntry(stackPointer, AT_PHDR, (uint64_t)process->SubBinary->ProgramHeaders, dryRun);
	}
	else
	{
		stackPointer = WriteAuxEntry(stackPointer, AT_ENTRY, process->Binary->Entry, dryRun);
		stackPointer = WriteAuxEntry(stackPointer, AT_BASE, process->Binary->BaseAddress, dryRun);
		stackPointer = WriteAuxEntry(stackPointer, AT_PAGESZ, 4096, dryRun);
		stackPointer = WriteAuxEntry(stackPointer, AT_PHNUM, process->Binary->ProgramHeaderCount, dryRun);
		stackPointer = WriteAuxEntry(stackPointer, AT_PHENT, process->Binary->ProgramHeaderEntrySize, dryRun);
		stackPointer = WriteAuxEntry(stackPointer, AT_PHDR, (uint64_t)process->Binary->ProgramHeaders, dryRun);
	}
	
	return stackPointer;
}

Process* CreateProcess(const char16_t* programName, const char16_t** argv, const char16_t** envp)
{
	Process* process = (Process*)rpmalloc(sizeof(Process));
	memset(process, 0, sizeof(Process));

	process->DefaultThreadStackSize = 128 * 1024;
	process->DefaultThreadStackBase = (uint64_t)VirtualAlloc(process->DefaultThreadStackSize, PrivilegeLevel::User);

	FillUnique((void*)process->DefaultThreadStackBase, 0x57a0000000000000, process->DefaultThreadStackSize);
	
	process->Binary = LoadElf(programName);

    if(!process->Binary)
    {
        return nullptr;
    }

    if(process->Binary->InterpreterFileHandle)
    {
        process->SubBinary = process->Binary;
        process->Binary = LoadElfFromHandle(process->Binary->InterpreterName, process->Binary->InterpreterFileHandle);
    }

	process->ProgramBreak = process->Binary->ProgramBreakLow;

	process->TLS = CreateUserModeTLS(process->Binary->TLSDataSize, process->Binary->TBSSSize, process->Binary->TLSData, process->Binary->TLSAlign);

	FillUnique((void*)process->TLS->FSBase, 0x7150000000000000, process->Binary->TLSDataSize + process->Binary->TBSSSize);

	//Prepare our stack pointer
	uint64_t* stackPointer = (uint64_t*)(process->DefaultThreadStackBase + process->DefaultThreadStackSize);
	uint64_t* stackPointerTop = stackPointer;
	uint64_t stackSize = process->DefaultThreadStackSize;

#define STACK_LEFT (stackSize - (stackPointerTop - stackPointer))

	//Data is written backwards on the stack in this order
	// End marker
	// Environment strings
	// Argument strings
	// Padding to 16b
	// Auxv
	// Envp
	// Argv

	//End marker
	*(--stackPointer) = 0;

	//Pre-calculate the size of the memory block so we can get args in the right order later ith less fuss
	uint8_t* stackPointerBytes = (uint8_t*)stackPointer;
	uint64_t stringBlockBytes = 0;
	int envc = 0;
	int argc = 0;
	const char16_t** argPointer = envp;

	argPointer = envp;
	while (*argPointer)
	{
		stringBlockBytes += wide_to_ascii(nullptr, *argPointer, STACK_LEFT) + 1;
		envc++;
		argPointer++;
	}

	argPointer = argv;
	while (*argPointer)
	{
		stringBlockBytes += wide_to_ascii(nullptr, *argPointer, STACK_LEFT) + 1;
		argc++;
		argPointer++;
	}

	//Align size up to 16b and ensure padding is initialized
	stringBlockBytes = (stringBlockBytes + 0xF) & ~0xF;
	*(uint64_t*)(stackPointerBytes - stringBlockBytes) = 0;

	//Write string data to the stack along with the pointers 

	//TODO Could stomp off the end here if strings above are too large
	uint64_t* stackPointerForAuxV = (uint64_t *)((uint8_t*)stackPointer - stringBlockBytes);
	uint64_t* stackPointerAfterAuxV = WriteProcessAuxVectors(process, stackPointerForAuxV, true /*dryRun*/, &process->TLS->Memory->StackCanary);
	stackPointer = stackPointerAfterAuxV;

	const char** envpOnStack = (const char** )(stackPointerAfterAuxV - (envc + 1));
	process->envp = envpOnStack;

	int writing;

	int envIndex = 0;
	argPointer = envp;
	envpOnStack[envc] = 0;
	while (*argPointer)
	{
		writing = wide_to_ascii(nullptr, *argPointer, STACK_LEFT);
		stackPointerBytes -= (writing + 1);
		writing = wide_to_ascii((char*)stackPointerBytes, *argPointer, STACK_LEFT);
		stackPointerBytes[writing] = 0;
		envpOnStack[envIndex] = (const char*)stackPointerBytes;
		envIndex++;
		argPointer++;
	}

	const char** argvOnStack = (const char** )(stackPointerAfterAuxV - (envc + argc + 2));
	stackPointer = ((uint64_t*)argvOnStack)-1;
	process->argv = argvOnStack;

	int argIndex = 0;
	argPointer = argv;
	argvOnStack[argc] = 0;
	while (*argPointer)
	{
		writing = wide_to_ascii(nullptr, *argPointer, STACK_LEFT);
		stackPointerBytes -= (writing + 1);
		writing = wide_to_ascii((char*)stackPointerBytes, *argPointer, STACK_LEFT);
		stackPointerBytes[writing] = 0;
		argvOnStack[argIndex] = (const char*)stackPointerBytes;
		argIndex++;
		argPointer++;
	}

	*(stackPointer) = argc;

#undef STACK_LEFT

	WriteProcessAuxVectors(process, stackPointerForAuxV, false /*dryRun*/, &process->TLS->Memory->StackCanary);

    // Set the stack pointer and argument count
    process->DefaultThreadStackStart = (uint64_t)stackPointer;

	ScheduleProcess(process);

	return process;

}

void DestroyProcess(Process* process)
{
	VirtualFree((void*)process->DefaultThreadStackBase, process->DefaultThreadStackSize);
	process->DefaultThreadStackBase = 0;

	DestroyTLS(process->TLS);
	process->TLS = nullptr;

	UnloadElf(process->Binary);
	process->Binary = nullptr;

	rpfree(process);
}

int RunProgram(const char16_t* programName, const char16_t** argv, const char16_t** envp)
{
	Process* process = CreateProcess(programName, argv, envp);

	DestroyProcess(process);

	return 0; //TODO
}
