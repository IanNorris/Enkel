#include "memory/memory.h"
#include "utilities/termination.h"
#include "kernel/init/gdt.h"
#include "kernel/init/tls.h"
#include "kernel/memory/state.h"
#include "memory/virtual.h"
#include "common/string.h"
#include <rpmalloc.h>
#include "kernel/user_mode/elf.h"

int GELFBinaryCount = 0;
ElfBinary** GELFBinaries = nullptr;

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

	uint64_t currentFSBase = GetFSBase();
	SetFSBase(process->TLS->FSBase);

	//TODO
	//process->Environment.argc, process->Environment.argv, process->Environment.envp

	SwitchToUserMode((uint64_t)process->DefaultThreadStackStart, process->Binary->Entry, userModeCodeSelector, userModeDataSelector);

	//Back in the kernel
	SetFSBase(currentFSBase);
}

Process* CreateProcess(const char16_t* programName)
{
	Process* process = (Process*)rpmalloc(sizeof(Process));
	memset(process, 0, sizeof(Process));

	process->DefaultThreadStackSize = 128 * 1024;
	process->DefaultThreadStackBase = (uint64_t)VirtualAlloc(process->DefaultThreadStackSize, PrivilegeLevel::User);
	
	process->Binary = LoadElf(programName);

	process->TLS = CreateUserModeTLS(process->Binary->TLSDataSize, process->Binary->TBSSSize, process->Binary->TLSData, process->Binary->TLSAlign);

	//Ensure we've got 
	uint64_t* stackTopPtr = (uint64_t*)(process->DefaultThreadStackBase + process->DefaultThreadStackSize);
	stackTopPtr -= 8; *stackTopPtr = 0;
	stackTopPtr -= 8; *stackTopPtr = 0;
	stackTopPtr -= 8; *stackTopPtr = 0;
	stackTopPtr -= 8; *stackTopPtr = 0;

	process->DefaultThreadStackStart = (uint64_t)stackTopPtr;

	const uint64_t envDataSize = 4096;

	//Prepare environment data.
	//TODO: Artificial limit to 4k of data
	uint64_t** processCommandArgs = (uint64_t**)VirtualAlloc(envDataSize, PrivilegeLevel::User);
	int argc = 3;
	int envc = 2;

	process->Environment.AllocatedAddress = processCommandArgs;
	process->Environment.AllocatedSize = envDataSize;
	process->Environment.argc = argc;
	process->Environment.argv = (char**)processCommandArgs;
	process->Environment.envp = (char**)&processCommandArgs[argc];

	const char* firstArg = "--mode";
	const char* secondArg = "awesome";

	const char* env1 = "OS_NAME=Enkel";
	const char* env2 = "LANGUAGE=en-GB";

	processCommandArgs[argc+envc] = 0;

	uint8_t* dataPointer = (uint8_t*)&processCommandArgs[argc+envc+2];
	size_t writing;

	int stringIndex = 0;

	//Arg 0 is the program name
	processCommandArgs[stringIndex++] = (uint64_t*)dataPointer;
	writing = wide_to_ascii((char*)dataPointer, programName, 256);
	dataPointer+=writing;

	//Arg 1
	processCommandArgs[stringIndex++] = (uint64_t*)dataPointer;
	writing = strlen(firstArg) + 1;
	memcpy(dataPointer, firstArg, writing);
	dataPointer+=writing;

	//Arg 2
	processCommandArgs[stringIndex++] = (uint64_t*)dataPointer;
	writing = strlen(secondArg) + 1;
	memcpy(dataPointer, secondArg, writing);
	dataPointer+=writing;

	//Env 0
	processCommandArgs[stringIndex++] = (uint64_t*)dataPointer;
	writing = strlen(env1) + 1;
	memcpy(dataPointer, env1, writing);
	dataPointer+=writing;

	//Env 2
	processCommandArgs[stringIndex++] = (uint64_t*)dataPointer;
	writing = strlen(env2) + 1;
	memcpy(dataPointer, env2, writing);
	dataPointer+=writing;

	ScheduleProcess(process);

	return process;

}

void DestroyProcess(Process* process)
{
	if(process->Environment.AllocatedAddress)
	{
		VirtualFree(process->Environment.AllocatedAddress, process->Environment.AllocatedSize);
		process->Environment.AllocatedAddress = nullptr;
	}

	VirtualFree((void*)process->DefaultThreadStackBase, process->DefaultThreadStackSize);
	process->DefaultThreadStackBase = 0;

	DestroyTLS(process->TLS);
	process->TLS = nullptr;

	UnloadElf(process->Binary);
	process->Binary = nullptr;

	rpfree(process);
}

void RunProgram(const char16_t* programName)
{
	Process* process = CreateProcess(programName);

	DestroyProcess(process);
}
