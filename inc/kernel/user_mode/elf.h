#pragma once

struct ElfSymbolExport
{
	uint8_t* SymbolName;
	uint64_t SymbolAddress;
};

struct ElfBinary
{
	char16_t Name[64];

	uint64_t BaseAddress;
	uint64_t AllocatedSize;
	uint64_t TextSection;
	uint64_t Entry;

	uint64_t RefCount;

	uint8_t* TLSData;
	uint64_t TLSDataSize;
	uint64_t TBSSSize;
	uint16_t TLSAlign;

	uint8_t* SymbolStringTable;
	uint64_t SymbolStringTableSize;
	uint64_t SymbolCount;
	ElfSymbolExport* Symbols;
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
	TLSAllocation* TLS;
};

void InitializeUserMode(void* fs);
ElfBinary* LoadElfFromMemory(const char16_t* programName, const uint8_t* elfStart);
ElfBinary* LoadElf(const char16_t* programName);
void UnloadElf(ElfBinary* elfBinary);
void ScheduleProcess(Process* process);
Process* CreateProcess(const char16_t* programName);
void RunProgram(const char16_t* programName);
