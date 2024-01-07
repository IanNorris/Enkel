#include "kernel/init/init.h"
#include "kernel/init/bootload.h"
#include "kernel/init/long_mode.h"
#include "kernel/console/console.h"
#include "memory/memory.h"
#include "kernel/memory/pml4.h"
#include "kernel/memory/state.h"
#include "common/string.h"
#include "utilities/termination.h"
#include "kernel/init/tls.h"
#include "utilities/qrdump.h"

#define PRINT_MEMORY_MAP 0

struct SPagingStructurePage
{
    union
    {
        uint64_t Entries[512]; // These entries take up exactly one page
        SPagingStructurePage* Next;
    };

}__attribute__((aligned(4096)));

//Extended spec flags (mostly mirrored from EFI_MEMORY_... in UefiSpec.h)
#define UEFI_MEMORY_UC              0x0000000000000001
#define UEFI_MEMORY_WC              0x0000000000000002
#define UEFI_MEMORY_WT              0x0000000000000004
#define UEFI_MEMORY_WB              0x0000000000000008
#define UEFI_MEMORY_UCE             0x0000000000000010
#define UEFI_MEMORY_WP              0x0000000000001000
#define UEFI_MEMORY_RP              0x0000000000002000
#define UEFI_MEMORY_XP              0x0000000000004000
#define UEFI_MEMORY_NV              0x0000000000008000
#define UEFI_MEMORY_MORE_RELIABLE   0x0000000000010000
#define UEFI_MEMORY_RO              0x0000000000020000
#define UEFI_MEMORY_SP              0x0000000000040000
#define UEFI_MEMORY_CPU_CRYPTO      0x0000000000080000
#define UEFI_MEMORY_RUNTIME         0x8000000000000000
#define UEFI_MEMORY_ISA_VALID       0x4000000000000000
#define UEFI_MEMORY_ISA_MASK        0x0FFFF00000000000

#define PDPT_ENTRIES 512
#define PD_ENTRIES 512
#define PT_ENTRIES (512*512)

#define WRITE_THROUGH (1ULL << 3)
#define CACHE_DISABLE (1ULL << 4)

#define PRESENT (1ULL<<0)
#define WRITABLE (1ULL<<1)
#define USER_CPL (1ULL<<2)
#define PAGE_LEVEL_WRITE_THROUGH (1ULL<<3)
#define PAGE_CACHE_DISABLE (1ULL<<4)
#define PAGE_WAS_ACCESSED (1ULL<<5)
#define PAGE_IS_DIRTY (1ULL<<6)
#define PAGE_PAT (1ULL<<7)
#define PAGE_2MB (1ULL<<7)
#define PAGE_GLOBAL (1ULL<<8)
#define NOT_EXECUTABLE (1ULL << 63)

#define PM_RESERVED_MASK 0xF000000000000
#define PML4_RESERVED_MASK 0xF000000000000 | 0x80

#define CHECK_PML4_RESERVED_BITS(entry, mask) _ASSERTF(((entry) & (mask)) == 0, "Reserved bits set")

static_assert(1 << PAGE_BITS == PAGE_SIZE, "Page size and page bits not consistent.");
static_assert(EFI_PAGE_SIZE == PAGE_SIZE, "Page sizes between kernel and EFI should match.");
static_assert(sizeof(SPagingStructurePage) == PAGE_SIZE, "Paging structure should be 1 page in size.");

constexpr uint64_t PML4AddressMask = (((1ULL << 52) - 1) & ~((1ULL << 12) - 1));

#define STATIC_PAGE_ENTRIES 2048

SPagingStructurePage* PagingFreePageHead = nullptr;

// Reserve space for the page tables
extern "C"
{
	SPagingStructurePage PML4 __attribute__((aligned(4096)));
}
SPagingStructurePage InitialPageTableEntries[STATIC_PAGE_ENTRIES] __attribute__((aligned(4096)));
bool PML4Set = false;

extern MemoryState PhysicalMemoryState;
extern MemoryState VirtualMemoryState;
void* NextFreePageTableEntriesBlock = nullptr;

const int NextPagingBlockSize = 0x1000;

void AllocateNextFreePageTableEntries()
{
	NextFreePageTableEntriesBlock = VirtualAlloc(sizeof(SPagingStructurePage) * NextPagingBlockSize, PrivilegeLevel::Kernel);

	memset(NextFreePageTableEntriesBlock, 0, sizeof(SPagingStructurePage) * NextPagingBlockSize);

	PhysicalMemoryState.InitDynamic();

	VirtualMemoryState.InitDynamic();
}

void PreparePML4FreeList(void* NextBlock)
{
    memset(NextBlock, 0, sizeof(InitialPageTableEntries));

    // Link all entries together to form the free list
    for (int i = STATIC_PAGE_ENTRIES - 1; i >= 0; i--)
    {
        InitialPageTableEntries[i].Next = PagingFreePageHead;
        PagingFreePageHead = &InitialPageTableEntries[i];
    }
}

SPagingStructurePage* GetPML4FreePage()
{
	if(PagingFreePageHead == nullptr)
	{
		SPagingStructurePage* NextPageSet = (SPagingStructurePage*)NextFreePageTableEntriesBlock;

		// Link all entries together to form the free list
		for (int i = NextPagingBlockSize - 1; i >= 0; i--)
		{
			NextPageSet[i].Next = PagingFreePageHead;
			PagingFreePageHead = &NextPageSet[i];
		}

		//Make sure we've got another ready to go
		//this will likely immediately consume several of the entries we've just created.
		AllocateNextFreePageTableEntries();
	}

	SPagingStructurePage* Result = PagingFreePageHead;
	PagingFreePageHead = PagingFreePageHead->Next;

    Result->Entries[0] = 0;

	memset(Result, 0, sizeof(SPagingStructurePage));

	return Result;
}

void FreePML4Page(SPagingStructurePage* Page)
{
	Page->Next = PagingFreePageHead;
	PagingFreePageHead = Page;
}

uint64_t GetPhysicalAddress(uint64_t virtualAddress, bool live)
{
    // Calculate indices
    uint64_t pml4Index = (virtualAddress >> 39) & 0x1FF;
    uint64_t pdptIndex = (virtualAddress >> 30) & 0x1FF;
    uint64_t pdIndex = (virtualAddress >> 21) & 0x1FF;
    uint64_t ptIndex = (virtualAddress >> 12) & 0x1FF;

	SPagingStructurePage* ActiveMap = live ? (SPagingStructurePage*)(GetCR3() & PML4AddressMask) : &PML4;

    // Access PML4
	uint64_t PML4Entry = ActiveMap->Entries[pml4Index];
    if (!(PML4Entry & PRESENT))
    {
        return INVALID_ADDRESS;
    }

    SPagingStructurePage* PDPT = (SPagingStructurePage*)(PML4Entry & PML4AddressMask);

    // Access PDPT
	uint64_t PDPTEntry = PDPT->Entries[pdptIndex];
    if (!(PDPTEntry & PRESENT))
    {
        return INVALID_ADDRESS;
    }

    SPagingStructurePage* PD = (SPagingStructurePage*)(PDPTEntry & PML4AddressMask);

    // Access PD
	uint64_t PDEntry = PD->Entries[pdIndex];
    if (!(PD->Entries[pdIndex] & PRESENT))
    {
        return INVALID_ADDRESS;
    }

	if(PDEntry & PAGE_2MB)
	{
		// Get the physical address
		uint64_t physicalAddress = PDEntry & PML4AddressMask;

		// Add the offset within the page
		physicalAddress += (virtualAddress & (PAGE_SIZE_2MB-1));

		return physicalAddress;
	}
	else
	{
		SPagingStructurePage* PT = (SPagingStructurePage*)(PDEntry & PML4AddressMask);

		// Access PT
		uint64_t PTEntry = PT->Entries[ptIndex];
		if (!(PTEntry & PRESENT))
		{
			return INVALID_ADDRESS;
		}

		// Get the physical address
		uint64_t physicalAddress = PTEntry & PML4AddressMask;

		// Add the offset within the page
		physicalAddress += (virtualAddress & (PAGE_SIZE-1));

		return physicalAddress;
	}
}

void MapPages(uint64_t virtualAddress, uint64_t physicalAddress, uint64_t size, bool writable, bool executable, PrivilegeLevel privilegeLevel, MemoryState::RangeState newState, PageFlags pageFlags)
{
    uint64_t originalVirtualAddress = virtualAddress;
    uint64_t originalPhysicalAddress = physicalAddress;

	_ASSERTF((size & (PAGE_SIZE-1)) == 0, "Misaligned page size");

    uint64_t endVirtualAddress = virtualAddress + size;

    virtualAddress &= PAGE_MASK;
    physicalAddress &= PAGE_MASK;

	_ASSERTF(originalVirtualAddress == virtualAddress, "Virtual address mismatch.");
	_ASSERTF(originalPhysicalAddress == physicalAddress, "Physical address mismatch.");

    uint64_t pageAlignedSize = (size + (PAGE_SIZE-1)) & PAGE_MASK;

    uint64_t physicalAddressEnd = physicalAddress + pageAlignedSize;

    PhysicalMemoryState.TagRange(physicalAddress, physicalAddressEnd, newState);
	VirtualMemoryState.TagRange(virtualAddress, endVirtualAddress, newState);

	while (virtualAddress < endVirtualAddress)
    {
		// Calculate indices
		uint64_t pml4Index = (virtualAddress >> 39) & 0x1FF;
		uint64_t pdptIndex = (virtualAddress >> 30) & 0x1FF;
		uint64_t pdIndex = (virtualAddress >> 21) & 0x1FF;
        uint64_t ptIndex = (virtualAddress >> 12) & 0x1FF;

        SPagingStructurePage* PDPT = nullptr;
        SPagingStructurePage* PD = nullptr;
        SPagingStructurePage* PT = nullptr;

		// Set up PML4
        if (PML4.Entries[pml4Index] & PRESENT)
        {
            PDPT = (SPagingStructurePage*)(PML4.Entries[pml4Index] & PML4AddressMask);
        }
        else
        {
            PDPT = GetPML4FreePage();
			PML4.Entries[pml4Index] = ((uint64_t)PDPT) | PRESENT | WRITABLE | USER_CPL;
			CHECK_PML4_RESERVED_BITS(PML4.Entries[pml4Index], PML4_RESERVED_MASK);
		}

		// Set up PDPT
		if (PDPT->Entries[pdptIndex] & PRESENT)
        {
            PD = (SPagingStructurePage*)(PDPT->Entries[pdptIndex] & PML4AddressMask);
        }
        else
        {
            PD = GetPML4FreePage();
            PDPT->Entries[pdptIndex] = ((uint64_t)PD) | PRESENT | WRITABLE | USER_CPL;
			CHECK_PML4_RESERVED_BITS(PDPT->Entries[pdptIndex], PM_RESERVED_MASK);
        }

		// Set up the PD
        if (PD->Entries[pdIndex] & PRESENT)
        {
			PT = (SPagingStructurePage*)(PD->Entries[pdIndex] & PML4AddressMask);
		}
        else
        {
			PT = GetPML4FreePage();
			PD->Entries[pdIndex] = ((uint64_t)PT) | PRESENT | WRITABLE | USER_CPL;
			CHECK_PML4_RESERVED_BITS(PD->Entries[pdIndex], PM_RESERVED_MASK);
		}

        if(newState == MemoryState::RangeState::Free)
        {
            PT->Entries[ptIndex] = (physicalAddress & PAGE_MASK);
        }
        else
        {
			bool wasPresent = (PT->Entries[ptIndex] & PRESENT) != 0;

			uint64_t userModeFlag = (privilegeLevel == PrivilegeLevel::User || (wasPresent && privilegeLevel == PrivilegeLevel::Keep && ((PT->Entries[ptIndex] & USER_CPL) == USER_CPL))) ? USER_CPL : 0;

			

			//We're about to yoink the memory away, flush the cache
			if(wasPresent)
			{
				asm volatile("clflush (%0)" ::"r"(virtualAddress) : "memory");
			}

            PT->Entries[ptIndex] = (physicalAddress & PAGE_MASK) | PRESENT | userModeFlag;
            if (writable)
            {
                PT->Entries[ptIndex] |= WRITABLE;
            }
            if (!executable)
            {
#if ENABLE_NX
                PT->Entries[ptIndex] |= NOT_EXECUTABLE;
#endif
            }

			if(pageFlags == PageFlags_Cache_Disable)
			{
				PT->Entries[ptIndex] |= CACHE_DISABLE;
			}

			if(pageFlags == PageFlags_Cache_WriteThrough)
			{
				PT->Entries[ptIndex] |= WRITE_THROUGH;
			}

			CHECK_PML4_RESERVED_BITS(PT->Entries[ptIndex], PM_RESERVED_MASK);

            uint64_t newPhysicalAddress = GetPhysicalAddress(virtualAddress, false);
            _ASSERTF(physicalAddress == newPhysicalAddress, "Physical address mismatch.");

			_ASSERTF(originalVirtualAddress == virtualAddress, "Virtual address mismatch.");
			_ASSERTF(originalPhysicalAddress == physicalAddress, "Physical address mismatch.");

			originalVirtualAddress += PAGE_SIZE;
			originalPhysicalAddress += PAGE_SIZE;
        }

		//Tell the CPU we've just invalidated that address.
        //TODO: Revisit this once we're looking at running user code.
		if(PML4Set)
        {
            asm volatile("invlpg (%0)" ::"r"(virtualAddress) : "memory");
        }

		// Move to the next page
		virtualAddress += PAGE_SIZE;
		physicalAddress += PAGE_SIZE;
	}
}

const char16_t* MemoryMapTypeToString(EFI_MEMORY_TYPE Type)
{
	switch(Type)
	{
		case EfiReservedMemoryType:
			return u"EfiReservedMemoryType";

		case EfiLoaderCode:
			return u"EfiLoaderCode";

		case EfiLoaderData:
			return u"EfiLoaderData";

		case EfiBootServicesCode:
			return u"EfiBootServicesCode";

		case EfiBootServicesData:
			return u"EfiBootServicesData";
			
		case EfiRuntimeServicesCode:
			return u"EfiRuntimeServicesCode";
			
		case EfiRuntimeServicesData:
			return u"EfiRuntimeServicesData";
			
		case EfiConventionalMemory:
			return u"EfiConventionalMemory";
			
		case EfiUnusableMemory:
			return u"EfiUnusableMemory";
			
		case EfiACPIReclaimMemory:
			return u"EfiACPIReclaimMemory";
			
		case EfiACPIMemoryNVS:
			return u"EfiACPIMemoryNVS";
			
		case EfiMemoryMappedIO:
			return u"EfiMemoryMappedIO";
			
		case EfiMemoryMappedIOPortSpace:
			return u"EfiMemoryMappedIOPortSpace";
			
		case EfiPalCode:
			return u"EfiPalCode";
			
		case EfiPersistentMemory:
			return u"EfiPersistentMemory";
			
		default:
			return u"EfiUnknown";
	}
}

void BuildPML4(KernelBootData* bootData)
{
    char16_t Buffer[32];

    KernelMemoryLayout& memoryLayout = bootData->MemoryLayout;

    uintptr_t HighestAddress = 0;
    for (uint32_t entry = 0; entry < memoryLayout.Entries; entry++)
    {
        const EFI_MEMORY_DESCRIPTOR& Desc = *((EFI_MEMORY_DESCRIPTOR*)((UINT8*)memoryLayout.Map + (entry * memoryLayout.DescriptorSize)));

        uintptr_t End = Desc.PhysicalStart + Desc.NumberOfPages * EFI_PAGE_SIZE;
        if (End > HighestAddress)
        {
            HighestAddress = End;
        }
    }

    const KernelMemoryLocation& binary = bootData->MemoryLayout.SpecialLocations[SpecialMemoryLocation_KernelBinary];
	const KernelMemoryLocation& bootstrap = bootData->MemoryLayout.SpecialLocations[SpecialMemoryLocation_APBootstrap];
    const KernelMemoryLocation& framebuffer = bootData->MemoryLayout.SpecialLocations[SpecialMemoryLocation_Framebuffer];
	const KernelMemoryLocation& stack = bootData->MemoryLayout.SpecialLocations[SpecialMemoryLocation_KernelStack];
	const KernelMemoryLocation& tables = bootData->MemoryLayout.SpecialLocations[SpecialMemoryLocation_Tables];

	#define PRINT_RANGE(block) LogPrintNumeric(u"Range of " #block u" From: ", block.PhysicalStart, u", "); LogPrintNumeric(u"To: ", block.PhysicalStart + block.ByteSize, u"\n")

	uint64_t PhysicalFramebuffer = GetPhysicalAddress(framebuffer.VirtualStart);

#if PRINT_MEMORY_MAP
	LogPrintNumeric(u"Framebuffer Virtual from: ", stack.VirtualStart, u"\n");
	LogPrintNumeric(u"Framebuffer Physical from: ", PhysicalFramebuffer, u"\n");

	PRINT_RANGE(binary);
	PRINT_RANGE(bootstrap);
	PRINT_RANGE(framebuffer);
	PRINT_RANGE(stack);
	PRINT_RANGE(tables);
#endif

	uint64_t HighestAddressUntrimmmed = HighestAddress;

	//Chop off any straggling bits so we get full pages only
	HighestAddress &= PAGE_MASK;

	PhysicalMemoryState.Init(0, HighestAddress);
	VirtualMemoryState.Init(1, PAGE_MASK); //Limit set by x86-64 architecture.

	memset(&PML4, 0, sizeof(PML4));
    PreparePML4FreeList(InitialPageTableEntries);
 
    _ASSERTF((uint64_t)&PML4 > binary.VirtualStart && (uint64_t)&PML4 < binary.VirtualStart + binary.ByteSize, "PML4 is not inside the kernel virtual range");

	uint64_t freeMemory = 0;

    for (uint32_t entry = 0; entry < memoryLayout.Entries; entry++)
    {
        EFI_MEMORY_DESCRIPTOR& Desc = *((EFI_MEMORY_DESCRIPTOR*)((UINT8*)memoryLayout.Map + (entry * memoryLayout.DescriptorSize)));
	
		bool IsFree =  Desc.Type == EfiConventionalMemory
					|| Desc.Type == EfiBootServicesCode
					|| Desc.Type == EfiBootServicesData;

		if(Desc.PhysicalStart == stack.PhysicalStart)
		{
			Desc.Attribute |= ENKEL_MEMORY_FLAG_STACK;
		}
		else if(Desc.PhysicalStart == tables.PhysicalStart)
		{
			Desc.Attribute |= ENKEL_MEMORY_FLAG_TABLES;
		}
		else if(Desc.PhysicalStart == bootstrap.PhysicalStart)
		{
			Desc.Attribute |= ENKEL_MEMORY_FLAG_BOOTSTRAP;
		}
		else if(Desc.PhysicalStart == (binary.PhysicalStart & PAGE_MASK))
		{
			Desc.Attribute |= ENKEL_MEMORY_FLAG_BINARY;
		}
		else if((framebuffer.PhysicalStart & PAGE_MASK) >= Desc.PhysicalStart && (framebuffer.PhysicalStart & PAGE_MASK) <= (Desc.PhysicalStart + (Desc.NumberOfPages * EFI_PAGE_SIZE)))
		{
			Desc.Attribute |= ENKEL_MEMORY_FLAG_FRAMEBUFFER;
		}

#if PRINT_MEMORY_MAP
		SerialPrint("Type: ");
		SerialPrint(MemoryMapTypeToString((EFI_MEMORY_TYPE)Desc.Type));
		SerialPrint(", ");

		LogPrintNumeric(u"FromP: ", Desc.PhysicalStart, u", ");
		LogPrintNumeric(u"ToP: ", Desc.PhysicalStart + (Desc.NumberOfPages * PAGE_SIZE), u", ");

		LogPrintNumeric(u"Pages: ", Desc.NumberOfPages, u", ");
		LogPrintNumeric(u"Bytes: ", Desc.NumberOfPages * PAGE_SIZE, u", ");

		bool PrevBit = false;
		if(Desc.Attribute & EFI_MEMORY_UC){ if(PrevBit){ SerialPrint("|"); } SerialPrint("UC"); PrevBit = true; }
		if(Desc.Attribute & EFI_MEMORY_WC){ if(PrevBit){ SerialPrint("|"); } SerialPrint("WC"); PrevBit = true; }
		if(Desc.Attribute & EFI_MEMORY_WT){ if(PrevBit){ SerialPrint("|"); } SerialPrint("WT"); PrevBit = true; }
		if(Desc.Attribute & EFI_MEMORY_WB){ if(PrevBit){ SerialPrint("|"); } SerialPrint("WB"); PrevBit = true; }
		if(Desc.Attribute & EFI_MEMORY_UCE){ if(PrevBit){ SerialPrint("|"); } SerialPrint("UCE"); PrevBit = true; }
		if(Desc.Attribute & EFI_MEMORY_WP){ if(PrevBit){ SerialPrint("|"); } SerialPrint("WP"); PrevBit = true; }
		if(Desc.Attribute & EFI_MEMORY_RP){ if(PrevBit){ SerialPrint("|"); } SerialPrint("RP"); PrevBit = true; }
		if(Desc.Attribute & EFI_MEMORY_XP){ if(PrevBit){ SerialPrint("|"); } SerialPrint("XP"); PrevBit = true; }
		if(Desc.Attribute & EFI_MEMORY_NV){ if(PrevBit){ SerialPrint("|"); } SerialPrint("NV"); PrevBit = true; }
		if(Desc.Attribute & EFI_MEMORY_MORE_RELIABLE){ if(PrevBit){ SerialPrint("|"); } SerialPrint("RELIABLE"); PrevBit = true; }
		if(Desc.Attribute & EFI_MEMORY_RO){ if(PrevBit){ SerialPrint("|"); } SerialPrint("RO"); PrevBit = true; }
		if(Desc.Attribute & UEFI_MEMORY_SP){ if(PrevBit){ SerialPrint("|"); } SerialPrint("SP"); PrevBit = true; }
		if(Desc.Attribute & UEFI_MEMORY_CPU_CRYPTO){ if(PrevBit){ SerialPrint("|"); } SerialPrint("CRYPTO"); PrevBit = true; }
		if(Desc.Attribute & EFI_MEMORY_RUNTIME){ if(PrevBit){ SerialPrint("|"); } SerialPrint("RUNTIME"); PrevBit = true; }
		if(Desc.Attribute & UEFI_MEMORY_ISA_VALID){ if(PrevBit){ SerialPrint("|"); } SerialPrint("ISA_VALID"); PrevBit = true; }
		if(Desc.Attribute & UEFI_MEMORY_ISA_MASK){ if(PrevBit){ SerialPrint("|"); } SerialPrint("ISA_MASK"); PrevBit = true; }
		if(Desc.Attribute & ENKEL_MEMORY_FLAG_STACK){ if(PrevBit){ SerialPrint("|"); } SerialPrint("E_STACK"); PrevBit = true; }
		if(Desc.Attribute & ENKEL_MEMORY_FLAG_TABLES){ if(PrevBit){ SerialPrint("|"); } SerialPrint("E_TABLES"); PrevBit = true; }
		if(Desc.Attribute & ENKEL_MEMORY_FLAG_BOOTSTRAP){ if(PrevBit){ SerialPrint("|"); } SerialPrint("E_BOOTSTRAP"); PrevBit = true; }
		if(Desc.Attribute & ENKEL_MEMORY_FLAG_FRAMEBUFFER){ if(PrevBit){ SerialPrint("|"); } SerialPrint("E_FRAMEBUFFER"); PrevBit = true; }
		if(Desc.Attribute & ENKEL_MEMORY_FLAG_BINARY){ if(PrevBit){ SerialPrint("|"); } SerialPrint("E_BINARY"); PrevBit = true; }

		SerialPrint("\n");
#endif

		uint64_t Size = Desc.NumberOfPages * EFI_PAGE_SIZE;

		bool IsReserved = 
				Desc.Type == EfiACPIMemoryNVS
			|| Desc.Type == EfiRuntimeServicesCode
			|| Desc.Type == EfiRuntimeServicesData
			|| Desc.Type == EfiACPIReclaimMemory
			|| Desc.Type == EfiUnusableMemory
			|| Desc.Type == EfiReservedMemoryType
			|| Desc.Type == EfiMemoryMappedIO
			|| Desc.Attribute & EFI_MEMORY_RUNTIME;

		uint64_t Start = Desc.PhysicalStart;
		uint64_t VirtualStart = Desc.VirtualStart;

		if(VirtualStart == 0)
		{
			VirtualStart = Desc.VirtualStart = Desc.PhysicalStart;
		}

		bool IsReadOnly = Desc.Type == EfiACPIReclaimMemory || Desc.Type == EfiACPIMemoryNVS || Desc.Type == EfiUnusableMemory || Desc.Attribute & EFI_MEMORY_RO;
		bool IsExecutable = Desc.Type == EfiRuntimeServicesCode || Desc.Type == EfiMemoryMapType_Kernel;

        if (!IsFree)
        {
            MapPages(VirtualStart, Start, Size, !IsReadOnly, IsExecutable, PrivilegeLevel::User, IsReserved ? MemoryState::RangeState::Reserved : MemoryState::RangeState::Used);
        }

		if(!IsReserved && IsFree)
		{
			freeMemory += Size;
		}
    }

	//Now we've built the map we need to find any gaps in the mapping.
	//Any gaps will be unaddressable.
	uint64_t StartP = 0x0;
	while(StartP != HighestAddressUntrimmmed)
    {
		bool found = false;
		for (uint32_t entry = 0; entry < memoryLayout.Entries; entry++)
    	{
			EFI_MEMORY_DESCRIPTOR& Desc = *((EFI_MEMORY_DESCRIPTOR*)((UINT8*)memoryLayout.Map + (entry * memoryLayout.DescriptorSize)));
			if(StartP == Desc.PhysicalStart)
			{
				StartP = Desc.PhysicalStart + (Desc.NumberOfPages * EFI_PAGE_SIZE);
				found = true;
				break;
			}
		}

		if(!found)
		{
			uint64_t lowest = ~0ULL;

			EFI_MEMORY_DESCRIPTOR* EntryDesc = nullptr;

			for (uint32_t entry = 0; entry < memoryLayout.Entries; entry++)
			{
				EFI_MEMORY_DESCRIPTOR& Desc = *((EFI_MEMORY_DESCRIPTOR*)((UINT8*)memoryLayout.Map + (entry * memoryLayout.DescriptorSize)));
				if(Desc.PhysicalStart > StartP && Desc.PhysicalStart < lowest)
				{
					lowest = Desc.PhysicalStart;
					EntryDesc = &Desc;
				}
			}

			if(lowest == ~0ULL)
			{
				_ASSERTF(false, "Unexpected memory map mismatch");
			}

			PhysicalMemoryState.TagRange(StartP, lowest, MemoryState::RangeState::Reserved);

#if PRINT_MEMORY_MAP
			LogPrintNumeric(u"Unmapped range from: ", StartP, u"");
			LogPrintNumeric(u" to: ", lowest, u"");

			LogPrintNumeric(u" (", (lowest-StartP), u" bytes)");

			if(EntryDesc->PhysicalStart == stack.PhysicalStart)
			{
				SerialPrint(u" STACK");
			}
			else if(EntryDesc->PhysicalStart == tables.PhysicalStart)
			{
				SerialPrint(u" TABLES");
			}
			else if(EntryDesc->PhysicalStart == bootstrap.PhysicalStart)
			{
				SerialPrint(u" BOOTSTRAP");
			}
			else if(EntryDesc->PhysicalStart == (binary.PhysicalStart & PAGE_MASK))
			{
				SerialPrint(u" BINARY");
			}
			else if((framebuffer.PhysicalStart & PAGE_MASK) >= EntryDesc->PhysicalStart && (framebuffer.PhysicalStart & PAGE_MASK) <= (EntryDesc->PhysicalStart + (EntryDesc->NumberOfPages * EFI_PAGE_SIZE)))
			{
				SerialPrint(u" FRAMEBUFFER");
			}

			SerialPrint(u"\n");
#endif
			
			StartP = lowest;
		}
    }

	uint64_t allocatedFrameBufferSize = (framebuffer.ByteSize + (PAGE_SIZE-1)) & PAGE_MASK;

    MapPages(framebuffer.VirtualStart, PhysicalFramebuffer, allocatedFrameBufferSize, true, false /*executable*/, PrivilegeLevel::User, MemoryState::RangeState::Reserved);
	freeMemory -= allocatedFrameBufferSize;

    ConsolePrint(u"Memory available: ");

	//This convoluted mess prints a decimal value of the memory in the system
	//with attached units. Eg 1.980GB or 3.999TB.
	const int KB = 1 * 1024;
	int precision = 2;
	bool unitFixed = false;
	bool dotPrinted = false;
	const char16_t* MemoryAvailableUnit = u"KB";

	while(precision != 0)
	{
		uint64_t MemoryAvailable = freeMemory / KB;
		uint64_t Units = KB;

		if (MemoryAvailable > 1024)
		{
			if(!unitFixed)
			{
				MemoryAvailableUnit = u"MB";
			}
			MemoryAvailable /= 1024;
			Units *= 1024;
		}

		if (MemoryAvailable > 1024)
		{
			if(!unitFixed)
			{
				MemoryAvailableUnit = u"GB";
			}
			MemoryAvailable /= 1024;
			Units *= 1024;
		}
		
		if (MemoryAvailable > 1024)
		{
			if(!unitFixed)
			{
				MemoryAvailableUnit = u"TB";
			}
			MemoryAvailable /= 1024;
			Units *= 1024;
		}

		if(MemoryAvailable == 0)
		{
			break;
		}

		if(unitFixed && !dotPrinted)
		{
			ConsolePrint(u".");
			dotPrinted = true;
		}

		witoabuf(Buffer, MemoryAvailable, 10);
		ConsolePrint(Buffer);
		unitFixed = true;

		freeMemory -= (MemoryAvailable * Units);
		precision--;
	}

	ConsolePrint(MemoryAvailableUnit);
    ConsolePrint(u"\n");
}

void MemCheck(KernelBootData* bootData)
{
	uint64_t HighestAddressWritten = 0;

	ConsolePrint(u"Memcheck...\n");
	KernelMemoryLayout& memoryLayout = bootData->MemoryLayout;

	uint64_t TargetVirtualPage = VirtualMemoryState.FindMinimumSizeFreeBlock(PAGE_SIZE);

	for (uint32_t entry = 0; entry < memoryLayout.Entries; entry++)
    {
        EFI_MEMORY_DESCRIPTOR& Desc = *((EFI_MEMORY_DESCRIPTOR*)((UINT8*)memoryLayout.Map + (entry * memoryLayout.DescriptorSize)));
	
		bool IsFree =  Desc.Type == EfiConventionalMemory
					|| Desc.Type == EfiBootServicesCode
					|| Desc.Type == EfiBootServicesData;

		uint64_t Start = Desc.PhysicalStart;
		uint64_t Size = Desc.NumberOfPages * EFI_PAGE_SIZE;

		bool IsReadOnly = Desc.Type == EfiACPIReclaimMemory || Desc.Type == EfiACPIMemoryNVS || Desc.Type == EfiUnusableMemory;

        if (IsFree)
		{
			if(!IsReadOnly)
			{
				for(uint64_t page = 0; page < Size / PAGE_SIZE; page+=10)
				{
					uint64_t PhysicalOffset = Start + (page * PAGE_SIZE);
					volatile uint64_t* WriteTarget = (uint64_t*)TargetVirtualPage;

					if(PhysicalMemoryState.GetPageState(PhysicalOffset) == MemoryState::RangeState::Free)
					{
						MapPages(TargetVirtualPage, PhysicalOffset, PAGE_SIZE, true, false, PrivilegeLevel::Kernel, MemoryState::RangeState::Used);
						*WriteTarget = 0xCDCDCDCDCDCDCDCD;
						if((uint64_t)WriteTarget > HighestAddressWritten)
						{
							HighestAddressWritten = (uint64_t)WriteTarget;
						}
						MapPages(TargetVirtualPage, PhysicalOffset, PAGE_SIZE, false, false, PrivilegeLevel::Kernel, MemoryState::RangeState::Free);
					}
				}
				
			}
		}
    }

	//Point it somewhere because our last free has meant we're not pointing to a physical page
	MapPages(TargetVirtualPage, 0, PAGE_SIZE, true, false, PrivilegeLevel::Kernel, MemoryState::RangeState::Used);
	VirtualFree((void*)TargetVirtualPage, PAGE_SIZE);

	LogPrintNumeric(u"Highest memcheck: ", HighestAddressWritten, u"\n");
}

#define PRINT_PML4 0

void BuildAndLoadPML4(KernelBootData* bootData)
{
#if PRINT_PML4
	char PML4Output[20000];
	PML4Output[0] = '\0';
	SetSerialTargetBuffer(PML4Output);
#endif

    PML4Set = false;
    BuildPML4(bootData);

#if PRINT_PML4
	QRDump(PML4Output);
	SetSerialTargetBuffer(nullptr);
	//HaltPermanently();
#endif

    ConsolePrint(u"Loading PML4...\n");
    LoadPageMapLevel4((uint64_t)&PML4);
    PML4Set = true;

    ConsolePrint(u"Now in long mode!...\n");
}
