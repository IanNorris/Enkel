#pragma once

struct ElfSymbolExport
{
	uint8_t* SymbolName;
	uint64_t SymbolAddress;
};

struct ElfBinary
{
	char16_t Name[64];
    char16_t InterpreterName[256];

	uint64_t BaseAddress;
	uint64_t AllocatedSize;
	uint64_t TextSection;
	uint64_t Entry;

	uint64_t RefCount;
    uint64_t InterpreterFileHandle;

	uint8_t* TLSData;
	uint64_t TLSDataSize;
	uint64_t TBSSSize;
	uint16_t TLSAlign;

	uint8_t* SymbolStringTable;
	uint64_t SymbolStringTableSize;
	uint64_t SymbolCount;
	ElfSymbolExport* Symbols;

	uint8_t* ProgramHeaders;
	uint32_t ProgramHeaderCount;
	uint32_t ProgramHeaderEntrySize;

	uint64_t ProgramBreakLow;
	uint64_t ProgramBreakHigh;
};

//Pointers are user mode relative for the process
//that we're creating.
struct EnvironmentData
{
	void* AllocatedAddress;
	uint64_t AllocatedSize;

	char** argv;
	char** envp;
	int argc;
};

struct Process
{
	uint64_t DefaultThreadStackStart;
	uint64_t DefaultThreadStackBase;
	uint64_t DefaultThreadStackSize;

	EnvironmentData Environment;

	ElfBinary* Binary;
    ElfBinary* SubBinary;
	TLSAllocation* TLS;

	uint64_t ProgramBreak;
};

void InitializeUserMode();
void ScheduleProcess(Process* process);
Process* CreateProcess(const char16_t* programName);
void RunProgram(const char16_t* programName);
