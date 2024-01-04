#include "memory/memory.h"
#include "utilities/termination.h"
#include "kernel/init/gdt.h"
#include "kernel/init/tls.h"
#include "kernel/console/console.h"
#include "kernel/memory/state.h"
#include "memory/virtual.h"
#include "common/string.h"

//Header guard to stop it pulling extra stuff in
#define __APPLE__
#include <elf.h>
#undef __APPLE__

#define FLAG_MASK_INTERRUPT 0x00000200
#define FLAG_MASK_DIRECTION 0x00000400
#define FLAG_MASK_TRAP 0x00000100
#define FLAG_MASK_ALIGNMENT 0x00040000
#define FLAG_MASK_NESTED_TASK 0x00004000

extern "C" void SwitchToUserMode(uint64_t stackPointer, uint64_t entry, uint16_t userModeCS, uint16_t userModeDS);

void OnBinaryLoadHook(uint64_t baseAddress, uint64_t segmentAddress, bool text)
{
	
}

// Function to validate and retrieve the entry point
void RunElf(const uint8_t* elfStart)
{
	Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)elfStart;
	Elf64_Phdr* segments = (Elf64_Phdr*)((const char*)elfStart + elfHeader->e_phoff);
	Elf64_Shdr* sections = (Elf64_Shdr*)((const char*)elfStart + elfHeader->e_shoff);

	//Verify the ELF is in the correct format

	if (elfHeader->e_ident[EI_MAG0] != ELFMAG0 ||
		elfHeader->e_ident[EI_MAG1] != ELFMAG1 ||
		elfHeader->e_ident[EI_MAG2] != ELFMAG2 ||
		elfHeader->e_ident[EI_MAG3] != ELFMAG3)
	{
		_ASSERTF(false, "Not an ELF file");
	}

	if (elfHeader->e_ident[EI_CLASS] != ELFCLASS64)
	{
		_ASSERTF(false, "Not an ELF64 file");
	}

	if (elfHeader->e_machine != EM_X86_64)
	{
		_ASSERTF(false, "Not an AMD64 file");
	}

	//We need to allocate memory for the kernel to be loaded into

	Elf64_Addr lowestAddress = ~0;
	Elf64_Addr highestAddress = 0;

	uint64_t tdataSize = 0; //Size of data to be copied into the TLS
	uint64_t tbssSize = 0; //Size of data to be zeroed in the TLS
	uint8_t* tdataStart = nullptr; //Start of the data to be copied into the TLS

	uint64_t tlsAlign = 0;

	for (int segmentHeader = 0; segmentHeader < elfHeader->e_phnum; segmentHeader++)
	{
		Elf64_Phdr& segment = segments[segmentHeader];

		if(segment.p_type == PT_TLS)
		{
			tdataSize = segment.p_filesz;
			tbssSize = segment.p_memsz - segment.p_filesz;
			tdataStart = (uint8_t*)elfStart + segment.p_offset;
			tlsAlign = segment.p_align;
		}

		if(segment.p_type != PT_LOAD)
		{
			continue;
		}

		if (segment.p_vaddr < lowestAddress)
		{
			lowestAddress = segment.p_vaddr;
		}

		if (segment.p_vaddr + segment.p_memsz > highestAddress)
		{
			highestAddress = segment.p_vaddr + segment.p_memsz;
		}		
	}

	uint64_t programSize = highestAddress - lowestAddress;

	programSize += PAGE_SIZE;
	programSize &= PAGE_MASK;

	uint64_t programStart = (uint64_t)VirtualAlloc(programSize, PrivilegeLevel::User);

	for (int segmentHeader = 0; segmentHeader < elfHeader->e_phnum; segmentHeader++)
	{
		Elf64_Phdr& segment = segments[segmentHeader];

		if (segment.p_type != PT_LOAD)
		{
			continue;
		}

		segment.p_vaddr += (Elf64_Addr)programStart;

		memcpy((void*)segment.p_vaddr, elfStart + segment.p_offset, segment.p_filesz);
		memset((uint8_t*)segment.p_vaddr + segment.p_filesz, 0, segment.p_memsz - segment.p_filesz);

		if(segment.p_flags & SHF_EXECINSTR)
		{
			VirtualProtect((uint8_t*)((uint64_t)segment.p_vaddr & PAGE_MASK), AlignSize(segment.p_memsz, 4096), MemoryProtection::Execute);
		}
		else if(segment.p_flags & SHF_WRITE)
		{
			VirtualProtect((uint8_t*)((uint64_t)segment.p_vaddr & PAGE_MASK), AlignSize(segment.p_memsz, 4096), MemoryProtection::ReadWrite);
		}
		else
		{
			VirtualProtect((uint8_t*)((uint64_t)segment.p_vaddr & PAGE_MASK), AlignSize(segment.p_memsz, 4096), MemoryProtection::ReadOnly);
		}
	}

	ConsolePrintNumeric(u"Base address: ", programStart, u"\n");

	const uint64_t stackSize = 128*1024;

	uint64_t stackStart = (uint64_t)VirtualAlloc(stackSize, PrivilegeLevel::User);
	uint64_t stackTop = stackStart + stackSize - 64;

	uint16_t umCS = ((uint16_t)GDTEntryIndex::UserCode * sizeof(GDTEntry)) | 0x3; // | ring3
	uint16_t umDS = ((uint16_t)GDTEntryIndex::UserData * sizeof(GDTEntry)) | 0x3;

	void* tlsBase = InitializeUserModeTLS(tdataSize, tbssSize, tdataStart, tlsAlign);

	SetFSBase((uint64_t)tlsBase);

	SwitchToUserMode(stackTop,programStart + elfHeader->e_entry, umCS, umDS);
}
