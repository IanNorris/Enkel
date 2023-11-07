#include "memory/memory.h"
#include "utilities/termination.h"
#include "kernel/console/console.h"
#include "kernel/memory/state.h"
#include "memory/virtual.h"

//Header guard to stop it pulling extra stuff in
#define __APPLE__
#include <elf.h>
#undef __APPLE__

typedef int (*MainFunction)();

// Function to validate and retrieve the entry point
void RunElf(const uint8_t* elfStart)
{
	Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)elfStart;
	Elf64_Phdr* programHeader = (Elf64_Phdr*)((const char*)elfStart + elfHeader->e_phoff);

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

	for (int sectionHeader = 0; sectionHeader < elfHeader->e_phnum; sectionHeader++)
	{
		Elf64_Phdr& section = programHeader[sectionHeader];

		if (section.p_type != PT_LOAD)
		{
			continue;
		}

		if (section.p_vaddr < lowestAddress)
		{
			lowestAddress = section.p_vaddr;
		}

		if (section.p_vaddr + section.p_memsz > highestAddress)
		{
			highestAddress = section.p_vaddr + section.p_memsz;
		}
	}

	uint64_t kernelSize = highestAddress - lowestAddress;

	kernelSize += PAGE_SIZE;
	kernelSize &= PAGE_MASK;

	uint64_t kernelStart = (uint64_t)VirtualAlloc(kernelSize);

	for (int sectionHeader = 0; sectionHeader < elfHeader->e_phnum; sectionHeader++)
	{
		Elf64_Phdr& section = programHeader[sectionHeader];

		if (section.p_type != PT_LOAD)
		{
			continue;
		}

		section.p_vaddr += (Elf64_Addr)kernelStart;

		memcpy((void*)section.p_vaddr, elfStart + section.p_offset, section.p_filesz);
		memset((uint8_t*)section.p_vaddr + section.p_filesz, 0, section.p_memsz - section.p_filesz);
	}

	MainFunction kernelEntry = (MainFunction)(kernelStart + elfHeader->e_entry);

	VirtualProtect((void*)kernelStart, kernelSize, MemoryProtection::Execute);

	kernelEntry();

	//return kernelEntry;
}
