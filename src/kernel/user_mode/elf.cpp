#include "memory/memory.h"
#include "utilities/termination.h"
#include "kernel/init/gdt.h"
#include "kernel/init/msr.h"
#include "kernel/console/console.h"
#include "kernel/memory/state.h"
#include "memory/virtual.h"

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

void sys_write(uint64_t fileHandle, void* data, uint64_t dataSize)
{
	SerialPrint((const char*)data);
}

void KERNEL_API SyscallEntry(uint64_t syscallIndex, void* param1, void* param2, void* param3, void* param4, void* param5)
{
	/*uint64_t syscallNumber;
    asm ("mov %%rax, %0" : "=r"(syscallNumber));

	switch(syscallIndex)
	{
		case 0x1:
			sys_write((uint64_t)param1, param2, (uint64_t)param3);
	}*/

	asm("sysret");
}

// Define the MSR constants
constexpr uint32_t IA32_STAR = 0xC0000081;
constexpr uint32_t IA32_LSTAR = 0xC0000082;
constexpr uint32_t IA32_FMASK = 0xC0000084;
constexpr uint32_t IA32_EFER = 0xC0000080;
constexpr uint64_t EFER_SCE = 0x0001;

void InitializeSyscalls()
{
    // Set system call entry point
    SetMSR(IA32_LSTAR, (uint64_t)&SyscallEntry);

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

	uint64_t programSize = highestAddress - lowestAddress;

	programSize += PAGE_SIZE;
	programSize &= PAGE_MASK;

	uint64_t programStart = (uint64_t)VirtualAlloc(programSize, PrivilegeLevel::User);

	for (int sectionHeader = 0; sectionHeader < elfHeader->e_phnum; sectionHeader++)
	{
		Elf64_Phdr& section = programHeader[sectionHeader];

		if (section.p_type != PT_LOAD)
		{
			continue;
		}

		section.p_vaddr += (Elf64_Addr)programStart;

		memcpy((void*)section.p_vaddr, elfStart + section.p_offset, section.p_filesz);
		memset((uint8_t*)section.p_vaddr + section.p_filesz, 0, section.p_memsz - section.p_filesz);
	}

	VirtualProtect((void*)programStart, programSize, MemoryProtection::Execute);

	const uint64_t stackSize = 128*1024;

	uint64_t stackStart = (uint64_t)VirtualAlloc(stackSize, PrivilegeLevel::User);
	uint64_t stackTop = stackStart + stackSize - 64;

	uint16_t umCS = ((uint16_t)GDTEntryIndex::UserCode * sizeof(GDTEntry)) | 0x3;
	uint16_t umDS = ((uint16_t)GDTEntryIndex::UserData * sizeof(GDTEntry)) | 0x3;

	SwitchToUserMode(stackTop,programStart + elfHeader->e_entry, umCS, umDS);
}
