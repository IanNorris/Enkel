#include "memory/memory.h"
#include "utilities/termination.h"
#include "kernel/init/gdt.h"
#include "kernel/init/tls.h"
#include "kernel/console/console.h"
#include "kernel/memory/state.h"
#include "memory/virtual.h"
#include "common/string.h"
#include "kernel/user_mode/elf.h"
#include <ff.h>
#include <rpmalloc.h>

//Header guard to stop it pulling extra stuff in
#define __APPLE__
#include <elf.h>
#undef __APPLE__

#define FLAG_MASK_INTERRUPT 0x00000200
#define FLAG_MASK_DIRECTION 0x00000400
#define FLAG_MASK_TRAP 0x00000100
#define FLAG_MASK_ALIGNMENT 0x00040000
#define FLAG_MASK_NESTED_TASK 0x00004000

FATFS* GUserModeFS = nullptr;
int GELFBinaryCount = 0;
ElfBinary** GELFBinaries = nullptr;

void InitializeUserMode(void* fsPtr)
{
	FATFS* fs = (FATFS*)fsPtr;

	GUserModeFS = fs;
	GELFBinaries = (ElfBinary**)rpmalloc(sizeof(ElfBinary*) * 16);
	GELFBinaryCount = 0;
}

extern "C" void SwitchToUserMode(uint64_t stackPointer, uint64_t entry, uint16_t userModeCS, uint16_t userModeDS);

extern "C" void __attribute__((used, noinline)) OnBinaryLoadHook_Inner()
{
    asm("nop");
}

extern "C" void __attribute__((used, noinline)) OnBinaryLoadHook(uint64_t baseAddress, uint64_t textSectionOffset, const char16_t* programName)
{
	OnBinaryLoadHook_Inner();

	asm("nop");
	asm("nop");
}

extern "C" void __attribute__((used, noinline)) OnBinaryUnloadHook_Inner()
{
    asm("nop");
}

extern "C" void __attribute__((used, noinline)) OnBinaryUnloadHook(uint64_t baseAddress, uint64_t textSectionOffset, const char16_t* programName)
{
	OnBinaryUnloadHook_Inner();

	asm("nop");
	asm("nop");
}

ElfBinary* LoadElfFromMemory(const char16_t* programName, const uint8_t* elfStart)
{
	for(int existing = 0; existing < GELFBinaryCount; existing++)
	{
		if(strcmp(GELFBinaries[existing]->Name, programName) == 0)
		{
			return GELFBinaries[existing];
		}
	}

	ElfBinary* elfBinary = (ElfBinary*)rpmalloc(sizeof(ElfBinary));
	memset(elfBinary, 0, sizeof(ElfBinary));

	strcpy(elfBinary->Name, programName);

	Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)elfStart;
	Elf64_Phdr* segments = (Elf64_Phdr*)((const char*)elfStart + elfHeader->e_phoff);
	Elf64_Shdr* sections = (Elf64_Shdr*)((const char*)elfStart + elfHeader->e_shoff);
	char* stringTable = (char*)(elfStart + sections[elfHeader->e_shstrndx].sh_offset);
	char* dynStrTable = nullptr;

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

	for (uint16_t i = 0; i < elfHeader->e_shnum; ++i)
    {
        Elf64_Shdr& section = sections[i];
		const char* sectionName = stringTable + section.sh_name;

        if (section.sh_type == SHT_STRTAB) // Check if it's a string table
        {
            // Compare section name with ".dynstr"
            const char* sectionName = (char*)(elfStart + sections[elfHeader->e_shstrndx].sh_offset + section.sh_name);
            if (strcmp(sectionName, ".dynstr") == 0)
            {
                dynStrTable = (char*)(elfStart + section.sh_offset);
                break;
            }
        }
	}

	for (uint16_t i = 0; i < elfHeader->e_shnum; ++i)
    {
        Elf64_Shdr& section = sections[i];
		const char* sectionName = stringTable + section.sh_name;

		if(section.sh_type == SHT_DYNAMIC)
		{
			Elf64_Dyn* dynamicSection = (Elf64_Dyn*)(elfStart + section.sh_offset);
            uint32_t entryCount = section.sh_size / section.sh_entsize;

            for (uint32_t j = 0; j < entryCount; ++j)
            {
                Elf64_Dyn& entry = dynamicSection[j];
                if (entry.d_tag != DT_NEEDED)
				{
                    break;
				}

                const char* libraryName = dynStrTable + entry.d_un.d_val;
				SerialPrint(libraryName);
				SerialPrint("\n");

				int wideIndex = 0;
				char16_t libraryWide[256];
				while(*libraryName)
				{
					libraryWide[wideIndex++] = *libraryName;
					libraryName++;
				}
				libraryWide[wideIndex] = 0;

                LoadElf(libraryWide);
            }
		}
	}

	for (int segmentHeader = 0; segmentHeader < elfHeader->e_phnum; segmentHeader++)
	{
		Elf64_Phdr& segment = segments[segmentHeader];

#if _DEBUG
		if(!(segment.p_type == PT_LOAD || segment.p_type == PT_TLS))
		{
			continue;
		}
#endif

		if (segment.p_vaddr < lowestAddress)
		{
			lowestAddress = segment.p_vaddr;
		}

		if (segment.p_vaddr + segment.p_memsz > highestAddress)
		{
			highestAddress = segment.p_vaddr + segment.p_memsz;
		}		
	}

	uint64_t programSize = highestAddress;
	programSize = AlignSize(programSize, PAGE_SIZE);

	uint64_t programStart = (uint64_t)VirtualAlloc(programSize, PrivilegeLevel::User);

	uint64_t text_section = 0;

	for (int segmentHeader = 0; segmentHeader < elfHeader->e_phnum; segmentHeader++)
	{
		Elf64_Phdr& segment = segments[segmentHeader];

		segment.p_vaddr = segment.p_vaddr + (Elf64_Addr)programStart;
		segment.p_paddr = segment.p_paddr + (Elf64_Addr)programStart;

		if(segment.p_type == PT_TLS)
		{
			tdataSize = segment.p_filesz;
			tbssSize = segment.p_memsz - segment.p_filesz;
			tdataStart = (uint8_t*)segment.p_vaddr;
			tlsAlign = segment.p_align;
		}

#if _DEBUG
		if(!(segment.p_type == PT_LOAD || segment.p_type == PT_TLS))
		{
			continue;
		}
#endif

		text_section = segment.p_vaddr + programStart;

		memcpy((void*)segment.p_vaddr, elfStart + segment.p_offset, segment.p_filesz);
		memset((uint8_t*)segment.p_vaddr + segment.p_filesz, 0, segment.p_memsz - segment.p_filesz);

#if ENABLE_NX
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
#endif
	}

#if _DEBUG
	OnBinaryLoadHook(programStart, text_section, programName);
#endif

	elfBinary->BaseAddress = programStart;
	elfBinary->AllocatedSize = programSize;

	elfBinary->TextSection = text_section;
	elfBinary->Entry = programStart + elfHeader->e_entry;

	elfBinary->TLSData = tdataStart;
	elfBinary->TLSDataSize = tdataSize;
	elfBinary->TBSSSize = tbssSize;
	elfBinary->TLSAlign = tlsAlign;

	elfBinary->RefCount = 1;

	GELFBinaries[GELFBinaryCount++] = elfBinary;

	return elfBinary;
}

ElfBinary* LoadElf(const char16_t* programName)
{
	char16_t filename[256];
	strcpy(filename, u"//");
	strcat(filename, programName);

	FIL file;

	FRESULT fr = f_open(&file, (const TCHAR*)filename, FA_READ);
	if(fr == FR_OK)
	{
		uint64_t fileSize = f_size(&file);

		uint64_t alignedSize = AlignSize(fileSize, 4096);

		uint8_t* buffer = (uint8_t*)VirtualAlloc(alignedSize,  PrivilegeLevel::User);
		UINT bytesRead;
		fr = f_read(&file, buffer, fileSize, &bytesRead);

		ElfBinary* binary = LoadElfFromMemory(programName, buffer);

		VirtualFree(buffer, alignedSize);

		return binary;
	}
	else
	{
		ConsolePrint(u"Failed to load ELF: ");
		ConsolePrint(filename);
		ConsolePrint(u"\n");
	}

	return nullptr;
}

void UnloadElf(ElfBinary* elfBinary)
{
	elfBinary->RefCount--;

	if(elfBinary->RefCount == 0)
	{
#if _DEBUG
		OnBinaryUnloadHook(elfBinary->BaseAddress, elfBinary->TextSection, elfBinary->Name);
#endif

		VirtualFree((void*)elfBinary->BaseAddress, elfBinary->AllocatedSize);

		rpfree(elfBinary);
	}
}

void ScheduleProcess(Process* process)
{
	ConsolePrintNumeric(u"Base address: ", process->Binary->BaseAddress, u"\n");

	const uint16_t userModeCodeSelector = ((uint16_t)GDTEntryIndex::UserCode * sizeof(GDTEntry)) | 0x3; // Selector | Ring 3
	const uint16_t userModeDataSelector = ((uint16_t)GDTEntryIndex::UserData * sizeof(GDTEntry)) | 0x3;

	uint64_t currentFSBase = GetFSBase();
	SetFSBase(process->TLS->FSBase);

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

void RunProgram(const char16_t* programName)
{
	Process* process = CreateProcess(programName);

	DestroyProcess(process);
}
