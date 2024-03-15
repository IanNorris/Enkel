#include "memory/memory.h"
#include "utilities/termination.h"
#include "kernel/init/gdt.h"
#include "kernel/init/tls.h"
#include "kernel/console/console.h"
#include "kernel/memory/state.h"
#include "memory/virtual.h"
#include "common/string.h"
#include "kernel/user_mode/elf.h"
#include "kernel/user_mode/ehframe.h"
#include <rpmalloc.h>

#include "fs/volume.h"

#define PROGRAM_BREAK_SIZE 64 * 1024 * 1024 //ProgramBreak is the amount allocated after the end of the binary to be used as a heap
											// It's called break

//Header guard to stop it pulling extra stuff in
#define __APPLE__
#include <elf.h>
#undef __APPLE__

#define FLAG_MASK_INTERRUPT 0x00000200
#define FLAG_MASK_DIRECTION 0x00000400
#define FLAG_MASK_TRAP 0x00000100
#define FLAG_MASK_ALIGNMENT 0x00040000
#define FLAG_MASK_NESTED_TASK 0x00004000

extern int GELFBinaryCount;
extern ElfBinary** GELFBinaries;

//TODO: Digest https://gist.github.com/x0nu11byt3/bcb35c3de461e5fb66173071a2379779

// NOTE: These look a bit weird, but we need to access the parameters from the debugger
// single stepping in the debugger is a bit weird, so instead we go up one stack frame,
// which means we need a dummy extra frame to step out of!

extern "C" void __attribute__((used, noinline)) OnBinaryLoadHook_Inner()
{
    asm("nop");
}

extern "C" void __attribute__((used, noinline)) OnBinaryLoadHook(uint64_t baseAddress, const char16_t* programName)
{
	OnBinaryLoadHook_Inner();
}

extern "C" void __attribute__((used, noinline)) OnBinaryUnloadHook_Inner()
{
    asm("nop");
}

extern "C" void __attribute__((used, noinline)) OnBinaryUnloadHook(uint64_t textSectionOffset, const char16_t* programName)
{
	OnBinaryUnloadHook_Inner();
}

void* ResolveSymbol(const char* name)
{
	for(int i = 0; i < GELFBinaryCount; i++)
	{
		ElfBinary* binary = GELFBinaries[i];

		for(int symbol = 0; symbol < (int)binary->SymbolCount; symbol++)
		{
			ElfSymbolExport& exportSymbol = binary->Symbols[symbol];

			if(strcmp((const char*)exportSymbol.SymbolName, name) == 0)
			{
				return (void*)(exportSymbol.SymbolAddress + binary->BaseAddress);
			}
		}
	}

	return nullptr;
}

void WriteRelocationA(Elf64_Rela& rel, const uint64_t programStart, const uint64_t gotStart, const uint64_t tlsStart, const char* dynamicStringTable, Elf64_Sym* symbols)
{
	// https://docs.oracle.com/cd/E19120-01/open.solaris/819-0690/chapter7-2/index.html

	uint64_t symIndex = ELF64_R_SYM(rel.r_info);
	uint64_t type = ELF64_R_TYPE(rel.r_info) & 0xffffffff;
	Elf64_Sym& sym = symbols[symIndex];
	Elf64_Addr* target = (Elf64_Addr*)(programStart + rel.r_offset);

	if (type == R_X86_64_GLOB_DAT || type == R_X86_64_64)
	{
		const char* name = dynamicStringTable + sym.st_name;

		uint64_t address = sym.st_value + programStart;
		if (sym.st_shndx == SHN_UNDEF)
		{
			address = (uint64_t)ResolveSymbol(name);
		}

		if(address == 0)
		{
			SerialPrint("Failed to resolve symbol: ");
			SerialPrint(name);
			SerialPrint(" @ 0x");
			char buffer[32];
			witoabuf(buffer, (uint64_t)rel.r_offset, 16, 0);
			SerialPrint(buffer);
			SerialPrint("\n");
		}

		*target = address + rel.r_addend;

		//*target = 0xFADF00D5;
	}
	else if (type == R_X86_64_JUMP_SLOT)
	{
		_ASSERTF(rel.r_addend == 0, "R_X86_64_JUMP_SLOT with non-zero addend");

		*target = sym.st_value;
	}
	else if(type == R_X86_64_RELATIVE)
	{
		*target = programStart + rel.r_addend;

		//*target = 0xEADF00D5;
	}
	else if(type == R_X86_64_IRELATIVE)
	{
    	uint64_t* offsetIndirect = (uint64_t*)(programStart + rel.r_offset);
		*target = (Elf64_Addr)(programStart + *offsetIndirect + rel.r_addend);

		//*target = 0xBADF00D5;
	}
	else if(type == R_X86_64_PC32)
	{
		int64_t relocation = static_cast<int64_t>(programStart + sym.st_value + rel.r_addend - (programStart + rel.r_offset));

		*(uint32_t*)target = static_cast<uint32_t>(relocation);

		//*target = 0xCADF00D5;
	}
	else if(type == R_X86_64_GOTOFF64)
	{
		_ASSERT(gotStart != 0);
		*target = rel.r_addend + gotStart + sym.st_value;

		//*target = 0xAADF00D5;
	}
	else if(type == R_X86_64_TPOFF64)
	{
		_ASSERT(tlsStart != 0);
		*target = rel.r_addend + tlsStart + sym.st_value;

		//*target = 0x0ADF00D5;
	}
	else
	{
		NOT_IMPLEMENTED();
	}
}

template<typename RelType>
void WriteRelocation(RelType& rel, const uint64_t programStart, const uint64_t gotStart, const uint64_t tlsStart, const char* dynamicStringTable, Elf64_Sym* symbols)
{
	// https://docs.oracle.com/cd/E19120-01/open.solaris/819-0690/chapter7-2/index.html

	uint64_t symIndex = ELF64_R_SYM(rel.r_info);
	uint64_t type = ELF64_R_TYPE(rel.r_info) & 0xffffffff;
	Elf64_Sym& sym = symbols[symIndex];
	Elf64_Addr* target = (Elf64_Addr*)(programStart + rel.r_offset);

	if (type == R_X86_64_GLOB_DAT || type == R_X86_64_JUMP_SLOT || type == R_X86_64_64)
	{
		*target = sym.st_value;
	}
	else if(type == R_X86_64_RELATIVE)
	{
		*target += programStart;
	}
	else if(type == R_X86_64_IRELATIVE)
	{
		uint64_t* offsetIndirect = (uint64_t*)(programStart + rel.r_offset);
		*target = programStart + (Elf64_Addr)*offsetIndirect;
	}
	else if(type == R_X86_64_PC32)
	{
		int64_t relocation = static_cast<int64_t>(sym.st_value - (programStart + rel.r_offset));
		*target = static_cast<uint32_t>(relocation);
	}
	else if(type == R_X86_64_GOTOFF64)
	{
		_ASSERT(gotStart != 0);
		*target = gotStart +  sym.st_value;
	}
	else if(type == R_X86_64_TPOFF64)
	{
		_ASSERT(tlsStart != 0);
		*target = tlsStart + sym.st_value;
	}
	else
	{
		NOT_IMPLEMENTED();
	}
}

ElfBinary* LoadElfFromMemory(const char16_t* programName, const uint8_t* elfStart)
{
	const bool performDynamicLinking = false;

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
	char* dynamicStringTable = nullptr;

	uint32_t dynamicSectionItemCount = 0;
	Elf64_Dyn* dynamicSection = nullptr;

	uint64_t symbolsItemCount = 0;
	Elf64_Sym* symbols = nullptr;

	uint64_t gotStart = 0;
	uint64_t tlsStart = 0;

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
            if (strcmp(sectionName, ".dynstr") == 0)
            {
                dynamicStringTable = (char*)(elfStart + section.sh_offset);
            }
        }
		else if(section.sh_type == SHT_DYNAMIC)
		{
			dynamicSection = (Elf64_Dyn*)(elfStart + section.sh_offset);
            dynamicSectionItemCount = section.sh_size / section.sh_entsize;
		}
		else if (section.sh_type == SHT_DYNSYM)
		{
			symbols = (Elf64_Sym*)(elfStart + section.sh_offset);
			symbolsItemCount = section.sh_size / section.sh_entsize;
        }
		else if(strcmp(sectionName, ".got") == 0)
		{
			gotStart = section.sh_offset + (uint64_t)elfStart;
		}
	}

	if(dynamicSection && performDynamicLinking)
	{
		for (uint32_t j = 0; j < dynamicSectionItemCount; ++j)
		{
			Elf64_Dyn& entry = dynamicSection[j];
			if (entry.d_tag != DT_NEEDED)
			{
				break;
			}

			const char* libraryName = dynamicStringTable + entry.d_un.d_val;
			SerialPrint("Loading dynamic library ");
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

	uint64_t breakSize = PROGRAM_BREAK_SIZE;
	uint64_t programSize = highestAddress;
	uint64_t allocatedSize = AlignSize(programSize + breakSize, PAGE_SIZE);

	_ASSERTF(allocatedSize != 0, "No loadable segments in ELF file");
	
	uint64_t programStart = (uint64_t)VirtualAlloc(allocatedSize, PrivilegeLevel::User);

	elfBinary->ProgramHeaderCount = elfHeader->e_phnum;
	elfBinary->ProgramHeaderEntrySize = sizeof(Elf64_Phdr);
	elfBinary->ProgramHeaders = (uint8_t*)programStart + elfHeader->e_phoff;
	memcpy(elfBinary->ProgramHeaders, segments, elfBinary->ProgramHeaderCount * elfBinary->ProgramHeaderEntrySize);

	//Update headers to the new copy
	segments = (Elf64_Phdr*)elfBinary->ProgramHeaders;

	elfBinary->ProgramBreakLow = programStart + programStart;
	elfBinary->ProgramBreakHigh = elfBinary->ProgramBreakLow + breakSize;

	uint64_t text_section = 0;

	for (int segmentHeader = 0; segmentHeader < elfHeader->e_phnum; segmentHeader++)
	{
		Elf64_Phdr& segment = segments[segmentHeader];

		uint64_t vaddr = segment.p_vaddr + (Elf64_Addr)programStart;

#if 0
		segment.p_vaddr = vaddr;
		segment.p_paddr = segment.p_paddr + (Elf64_Addr)programStart;
#endif

		if(segment.p_type == PT_TLS)
		{
			if(tlsStart == 0)
			{
				tlsStart = vaddr;
			}

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

		text_section = vaddr;

		memcpy((void*)vaddr, elfStart + segment.p_offset, segment.p_filesz);
		memset((uint8_t*)vaddr + segment.p_filesz, 0, segment.p_memsz - segment.p_filesz);
	}

#if 0
	if (symbols != nullptr)
	{
		uint64_t totalStringLength = 0;
		uint64_t symbolCount = 0;

		for (Elf32_Word i = 0; i < symbolsItemCount; ++i)
		{
            Elf64_Sym& sym = symbols[i];
			const char* name = dynamicStringTable + sym.st_name;

			uint64_t currentSymbolNameLength = strlen(name);

            if((ELF64_ST_BIND(sym.st_info) != STB_LOCAL) && currentSymbolNameLength)
			{
				totalStringLength += currentSymbolNameLength + 1;
				symbolCount++;
			}
        }

		elfBinary->SymbolCount = 0;
		elfBinary->SymbolStringTableSize = totalStringLength;
		elfBinary->SymbolStringTable = (uint8_t*)rpmalloc(totalStringLength);
		elfBinary->Symbols = (ElfSymbolExport*)rpmalloc(sizeof(ElfSymbolExport) * symbolCount);

		uint8_t* currentStringTableOffset = elfBinary->SymbolStringTable;

        for (Elf32_Word i = 0; i < symbolsItemCount; ++i)
		{
            Elf64_Sym& sym = symbols[i];
			const char* name = dynamicStringTable + sym.st_name;

			uint64_t currentSymbolNameLength = strlen(name);

            if (sym.st_shndx == SHN_UNDEF && currentSymbolNameLength)
			{
                void* addr = ResolveSymbol(name);

                if (addr == nullptr)
				{
                    // Handle unresolved symbol
                    SerialPrint("Unresolved symbol: ");
                    SerialPrint(name);
					SerialPrint("\n");
					addr = (void*)0xBADF00D5;
                }
				else
				{
					SerialPrint("Resolved symbol: ");
                    SerialPrint(name);
					SerialPrint(" @ 0x");
					char buffer[32];
					witoabuf(buffer, (uint64_t)addr, 16, 0);
					SerialPrint(buffer);
					SerialPrint("\n");
				}

                sym.st_value = (Elf64_Addr)addr;
            }
			else if((ELF64_ST_BIND(sym.st_info) != STB_LOCAL) && currentSymbolNameLength)
			{
				ElfSymbolExport& exportSymbol = elfBinary->Symbols[elfBinary->SymbolCount++];
				exportSymbol.SymbolName = currentStringTableOffset;
				exportSymbol.SymbolAddress = sym.st_value;

				strcpy((char*)currentStringTableOffset, name);
				currentStringTableOffset += currentSymbolNameLength + 1;
			}
        }
    }

	for (uint16_t i = 0; i < elfHeader->e_shnum; ++i)
    {
        Elf64_Shdr& section = sections[i];
		const char* sectionName = stringTable + section.sh_name;

		if (section.sh_type == SHT_RELA)
		{
            Elf64_Rela* relas = (Elf64_Rela*)(elfStart + section.sh_offset);
			for (Elf64_Xword i = 0; i < section.sh_size / section.sh_entsize; ++i)
			{
				Elf64_Rela& rela = relas[i];

				WriteRelocationA(rela, programStart, gotStart, tlsStart, dynamicStringTable, symbols);
			}
        }
		else if (section.sh_type == SHT_REL)
		{
            Elf64_Rel* rels = (Elf64_Rel*)(elfStart + section.sh_offset);
			for (Elf64_Xword i = 0; i < section.sh_size / section.sh_entsize; ++i)
			{
				Elf64_Rel& rel = rels[i];
				WriteRelocation(rel, programStart, gotStart, tlsStart, dynamicStringTable, symbols);
			}
        }
		else if (strcmp(sectionName, ".eh_frame") == 0)
		{
			/*uint8_t* ehFrame = (uint8_t*)(elfStart + section.sh_offset);
			
			uint8_t* current = ehFrame;
			while (current < ehFrame + section.sh_size)
			{
				uint32_t length = *reinterpret_cast<uint32_t*>(current);
				uint8_t* nextRecord = current + 4 + length;

				if (length == 0)
				{
					break;
				}

				uint64_t cieId = *reinterpret_cast<uint32_t*>(current + 4);
				if (cieId == 0) // This is a CIE
				{ 
					// CIEs don't typically contain pointers needing patching
				}
				else
				{
					// This is an FDE

					uint64_t* fdePtr = reinterpret_cast<uint64_t*>(current + 8);
					*fdePtr += programStart; // Patch the FDE pointer
				}

				current = nextRecord;
			}

			_ASSERTF(current == ehFrame + section.sh_size, "Didn't parse the entire .eh_frame section");*/
		}
		else if (strcmp(sectionName, ".eh_frame_hdr") == 0)
		{
			Elf64_EhFrameHdr_Header* ehFrameHdrHeader = (Elf64_EhFrameHdr_Header*)(programStart + section.sh_offset);

			Elf64_EhFrameHdrEntry* ehFrameHdr = (Elf64_EhFrameHdrEntry*)(ehFrameHdrHeader+1);

			for (uint32_t i = 0; i < ehFrameHdrHeader->TableEntryCount; ++i)
			{
				ehFrameHdr->FDEAddress += programStart;
				ehFrameHdr->StartAddress += programStart;

				ehFrameHdr++;
			}
		}
	}
#endif

	//Do this last, as we can't modify most of the segments once the protection flags are configured.
	for (int segmentHeader = 0; segmentHeader < elfHeader->e_phnum; segmentHeader++)
	{
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

	/*for (uint16_t i = 0; i < elfHeader->e_shnum; ++i)
    {
        Elf64_Shdr& section = sections[i];
		//const char* sectionName = stringTable + section.sh_name;

		typedef void (*InitFunction)(void);

		if (section.sh_type == SHT_PREINIT_ARRAY)
		{
			uint64_t entryCount = section.sh_size /= sizeof(InitFunction);

			//TODO: DO NOT CALL THIS FROM THE KERNEL!!!!!
			uint64_t* initFunctions = (uint64_t*)(programStart + section.sh_addr);

			for (uint64_t i = 0; i < entryCount; ++i)
			{
				InitFunction* function = (InitFunction*)(initFunctions[i] + programStart);
				(*function)();
			}
		}
		//TODO .fini_array
	}

	for (uint16_t i = 0; i < elfHeader->e_shnum; ++i)
    {
        Elf64_Shdr& section = sections[i];
		//const char* sectionName = stringTable + section.sh_name;

		typedef void (*InitFunction)(void);

		if (section.sh_type == SHT_INIT_ARRAY)
		{
			uint64_t entryCount = section.sh_size /= sizeof(InitFunction);

			//TODO: DO NOT CALL THIS FROM THE KERNEL!!!!!
			InitFunction* initFunctions = (InitFunction*)(programStart + section.sh_addr);

			for (uint64_t i = 0; i < entryCount; ++i)
			{
				initFunctions[i]();
			}
		}
		//TODO .fini_array
	}*/

	const char* interpreter = nullptr;

	for (uint16_t i = 0; i < elfHeader->e_shnum; ++i)
    {
        Elf64_Shdr& section = sections[i];
		const char* sectionName = stringTable + section.sh_name;

		if (strcmp(sectionName, ".interp") == 0)
		{
			interpreter = (const char*)(elfStart + section.sh_offset);

			SerialPrint("Interpreter: ");
			SerialPrint(interpreter);
			SerialPrint("\n");

			ascii_to_wide(elfBinary->InterpreterName, interpreter, MAX_PATH);

			elfBinary->InterpreterFileHandle = VolumeOpenHandle(0, elfBinary->InterpreterName, 0);
			if(elfBinary->InterpreterFileHandle == 0)
			{
				UnloadElf(elfBinary);
				return nullptr;
			}
		}
	}

#if _DEBUG
	OnBinaryLoadHook(programStart, programName);
#endif

	elfBinary->BaseAddress = programStart;
	elfBinary->AllocatedSize = allocatedSize;

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

ElfBinary* LoadElfFromHandle(const char16_t* programName, VolumeFileHandle handle)
{
	if(handle != 0)
	{
		uint64_t fileSize = VolumeGetSize(handle);

		uint64_t alignedSize = AlignSize(fileSize, 4096);

		uint8_t* buffer = (uint8_t*)VirtualAlloc(alignedSize,  PrivilegeLevel::User);
		FillUnique(buffer, 0xe1fb000000000000, alignedSize);
		
		uint64_t bytesRead = VolumeRead(handle, 0, buffer, fileSize);

		ElfBinary* binary = LoadElfFromMemory(programName, buffer);

		ConsolePrint(programName);
		ConsolePrintNumeric(u" loaded at ", binary->BaseAddress, u"");
		ConsolePrintNumeric(u"-", binary->BaseAddress + binary->AllocatedSize, u"\n");

		VirtualFree(buffer, alignedSize);

		return binary;
	}
	else
	{
		ConsolePrint(u"Failed to load ELF: ");
		ConsolePrint(programName);
		ConsolePrint(u"\n");
	}

	return nullptr;
}

ElfBinary* LoadElf(const char16_t* programName)
{
	VolumeFileHandle handle = VolumeOpenHandle(0, programName, 0);
	if(handle != 0)
	{
		ElfBinary* binary = LoadElfFromHandle(programName, handle);

		VolumeCloseHandle(handle);

		return binary;
	}
	else
	{
		ConsolePrint(u"Failed to load ELF: ");
		ConsolePrint(programName);
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
		OnBinaryUnloadHook(elfBinary->TextSection, elfBinary->Name);
#endif

		VirtualFree((void*)elfBinary->BaseAddress, elfBinary->AllocatedSize);

		rpfree(elfBinary);
	}
}
