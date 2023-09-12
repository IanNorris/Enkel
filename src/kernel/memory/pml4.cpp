#include "kernel/init/init.h"
#include "kernel/init/bootload.h"
#include "kernel/init/long_mode.h"
#include "kernel/console/console.h"
#include "memory/memory.h"
#include "kernel/memory/pml4.h"
#include "kernel/memory/state.h"
#include "common/string.h"
#include "utilities/termination.h"

struct SPagingStructurePage
{
    union
    {
        uint64_t Entries[512]; // These entries take up exactly one page
        SPagingStructurePage* Next;
    };

}__attribute__((aligned(4096)));

#define PDPT_ENTRIES 512
#define PD_ENTRIES 512
#define PT_ENTRIES (512*512)

#define WRITE_THROUGH (1ULL << 3)
#define CACHE_DISABLE (1ULL << 4)

#define PRESENT 1
#define WRITABLE 2
#define NOT_EXECUTABLE (1ULL << 63)

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

void PreparePML4FreeList()
{
    memset(&PML4, 0, sizeof(PML4));
    memset(InitialPageTableEntries, 0, sizeof(InitialPageTableEntries));

    // Link all entries together to form the free list
    for (int i = STATIC_PAGE_ENTRIES - 1; i >= 0; i--)
    {
        InitialPageTableEntries[i].Next = PagingFreePageHead;
        PagingFreePageHead = &InitialPageTableEntries[i];
    }
}

SPagingStructurePage* GetPML4FreePage()
{
	SPagingStructurePage* Result = PagingFreePageHead;
	PagingFreePageHead = PagingFreePageHead->Next;

    Result->Entries[0] = 0;

	return Result;
}

void FreePML4Page(SPagingStructurePage* Page)
{
    memset(Page, 0, sizeof(SPagingStructurePage));

	Page->Next = PagingFreePageHead;
	PagingFreePageHead = Page;
}

uint64_t GetPhysicalAddress(uint64_t virtualAddress)
{
    // Calculate indices
    uint64_t pml4Index = (virtualAddress >> 39) & 0x1FF;
    uint64_t pdptIndex = (virtualAddress >> 30) & 0x1FF;
    uint64_t pdIndex = (virtualAddress >> 21) & 0x1FF;
    uint64_t ptIndex = (virtualAddress >> 12) & 0x1FF;

    // Access PML4
    if (!(PML4.Entries[pml4Index] & PRESENT))
    {
        return INVALID_ADDRESS;
    }

    SPagingStructurePage* PDPT = (SPagingStructurePage*)(PML4.Entries[pml4Index] & PML4AddressMask);

    // Access PDPT
    if (!(PDPT->Entries[pdptIndex] & PRESENT))
    {
        return INVALID_ADDRESS;
    }

    SPagingStructurePage* PD = (SPagingStructurePage*)(PDPT->Entries[pdptIndex] & PML4AddressMask);

    // Access PD
    if (!(PD->Entries[pdIndex] & PRESENT))
    {
        return INVALID_ADDRESS;
    }

    SPagingStructurePage* PT = (SPagingStructurePage*)(PD->Entries[pdIndex] & PML4AddressMask);

    // Access PT
    if (!(PT->Entries[ptIndex] & PRESENT))
    {
        return INVALID_ADDRESS;
    }

    // Get the physical address
    uint64_t physicalAddress = PT->Entries[ptIndex] & PML4AddressMask;

    // Add the offset within the page
    physicalAddress += (virtualAddress & (PAGE_SIZE-1));

    return physicalAddress;
}

void MapPages(uint64_t virtualAddress, uint64_t physicalAddress, uint64_t size, bool writable, bool executable, MemoryState::RangeState newState, PageFlags pageFlags)
{
    uint64_t originalVirtualAddress = virtualAddress;
    uint64_t originalPhysicalAddress = physicalAddress;

	_ASSERTF((size & (PAGE_SIZE-1)) == 0, "Misaligned page size");

    uint64_t endVirtualAddress = virtualAddress + size;

    virtualAddress &= PAGE_MASK;
    physicalAddress &= PAGE_MASK;

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
			PML4.Entries[pml4Index] = ((uint64_t)PDPT) | PRESENT | WRITABLE;
		}

		// Set up PDPT
		if (PDPT->Entries[pdptIndex] & PRESENT)
        {
            PD = (SPagingStructurePage*)(PDPT->Entries[pdptIndex] & PML4AddressMask);
        }
        else
        {
            PD = GetPML4FreePage();
            PDPT->Entries[pdptIndex] = ((uint64_t)PD) | PRESENT | WRITABLE;
        }

		// Set up the PD
        if (PD->Entries[pdIndex] & PRESENT)
        {
			PT = (SPagingStructurePage*)(PD->Entries[pdIndex] & PML4AddressMask);
		}
        else
        {
			PT = GetPML4FreePage();
			PD->Entries[pdIndex] = ((uint64_t)PT) | PRESENT | WRITABLE;
		}

        if(newState == MemoryState::RangeState::Free)
        {
            PT->Entries[ptIndex] = (physicalAddress & PAGE_MASK);
        }
        else
        {
			bool wasPresent = (PT->Entries[ptIndex] & PRESENT) != 0;

			//We're about to yoink the memory away, flush the cache
			if(wasPresent)
			{
				asm volatile("clflush (%0)" ::"r"(virtualAddress) : "memory");
			}

            PT->Entries[ptIndex] = (physicalAddress & PAGE_MASK) | PRESENT;
            if (writable)
            {
                PT->Entries[ptIndex] |= WRITABLE;
            }
            if (!executable)
            {
                PT->Entries[ptIndex] |= NOT_EXECUTABLE;
            }

			if(pageFlags == PageFlags_Cache_Disable)
			{
				PT->Entries[ptIndex] |= CACHE_DISABLE;
			}

			if(pageFlags == PageFlags_Cache_WriteThrough)
			{
				PT->Entries[ptIndex] |= WRITE_THROUGH;
			}

            uint64_t newPhysicalAddress = GetPhysicalAddress(virtualAddress);
            _ASSERTF(physicalAddress == newPhysicalAddress, "Physical address mismatch.");
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

    uint64_t MemoryAvailable = HighestAddress / (1 * 1024 * 1024);
    const char16_t* MemoryAvailableUnit = u"MB";

    if (MemoryAvailable > 1024)
    {
        MemoryAvailableUnit = u"GB";
        MemoryAvailable /= 1024;
    }

    if (MemoryAvailable > 1024)
    {
        MemoryAvailableUnit = u"TB";
        MemoryAvailable /= 1024;
    }

    ConsolePrint(u"Memory available: ");
    witoabuf(Buffer, MemoryAvailable, 10);
    ConsolePrint(Buffer);
    ConsolePrint(MemoryAvailableUnit);
    ConsolePrint(u"\n");

    const KernelMemoryLocation& stack = bootData->MemoryLayout.SpecialLocations[SpecialMemoryLocation_KernelStack];
    const KernelMemoryLocation& binary = bootData->MemoryLayout.SpecialLocations[SpecialMemoryLocation_KernelBinary];
    const KernelMemoryLocation& framebuffer = bootData->MemoryLayout.SpecialLocations[SpecialMemoryLocation_Framebuffer];

    if(stack.PhysicalStart + stack.ByteSize > HighestAddress)
    {
        HighestAddress = ((stack.PhysicalStart + stack.ByteSize + (PAGE_SIZE-1))) & PAGE_MASK;
    }

    if(binary.PhysicalStart + binary.ByteSize > HighestAddress)
    {
        HighestAddress = ((binary.PhysicalStart + binary.ByteSize + (PAGE_SIZE-1))) & PAGE_MASK;
    }

    if(framebuffer.PhysicalStart + framebuffer.ByteSize > HighestAddress)
    {
        HighestAddress = ((framebuffer.PhysicalStart + framebuffer.ByteSize + (PAGE_SIZE-1))) & PAGE_MASK;
    }

	//Chop off any straggling bits so we get full pages only
	HighestAddress &= PAGE_MASK;

	PhysicalMemoryState.Init(HighestAddress);
	VirtualMemoryState.Init(2ULL << 48); //Limit set by x86-64 architecture.

    PreparePML4FreeList();
 
    _ASSERTF((uint64_t)&PML4 > binary.VirtualStart && (uint64_t)&PML4 < binary.VirtualStart + binary.ByteSize, "PML4 is not inside the kernel virtual range");

    for (uint32_t entry = 0; entry < memoryLayout.Entries; entry++)
    {
        EFI_MEMORY_DESCRIPTOR& Desc = *((EFI_MEMORY_DESCRIPTOR*)((UINT8*)memoryLayout.Map + (entry * memoryLayout.DescriptorSize)));
	
		bool IsFree =  Desc.Type == EfiConventionalMemory
					|| Desc.Type == EfiBootServicesCode
					|| Desc.Type == EfiBootServicesData;

        if (!IsFree)
        {
            uint64_t Start = Desc.PhysicalStart;
            uint64_t End = Desc.PhysicalStart + Desc.NumberOfPages * EFI_PAGE_SIZE;
            uint64_t VirtualStart = Desc.VirtualStart;
            uint64_t VirtualEnd = VirtualStart + Desc.NumberOfPages * EFI_PAGE_SIZE;

			if(VirtualStart == 0)
			{
				VirtualStart = Desc.VirtualStart = Desc.PhysicalStart;
			}

            uint64_t Size = Desc.NumberOfPages * EFI_PAGE_SIZE;

            bool IsReserved = 
                   Desc.Type == EfiACPIMemoryNVS
                || Desc.Type == EfiBootServicesData
                || Desc.Type == EfiBootServicesCode
                || Desc.Type == EfiRuntimeServicesCode
                || Desc.Type == EfiRuntimeServicesData
                || Desc.Type == EfiACPIReclaimMemory
				|| Desc.Type == EfiUnusableMemory
                || Desc.Type == EfiReservedMemoryType;

			bool IsReadOnly = Desc.Type == EfiACPIReclaimMemory || Desc.Type == EfiACPIMemoryNVS || Desc.Type == EfiUnusableMemory;
			bool IsExecutable = Desc.Type == EfiRuntimeServicesCode || Desc.Type == EfiMemoryMapType_Kernel;

            MapPages(VirtualStart, Start, Size, !IsReadOnly, IsExecutable, IsReserved ? MemoryState::RangeState::Reserved : MemoryState::RangeState::Used);
        }
    }

	uint64_t allocatedFrameBufferSize = (framebuffer.ByteSize + (PAGE_SIZE-1)) & PAGE_MASK;

    MapPages(framebuffer.VirtualStart, framebuffer.PhysicalStart, allocatedFrameBufferSize, true, false /*executable*/, MemoryState::RangeState::Used);
}

void BuildAndLoadPML4(KernelBootData* bootData)
{
    PML4Set = false;
    BuildPML4(bootData);

    ConsolePrint(u"Loading PML4...\n");
    LoadPageMapLevel4((uint64_t)&PML4);
    PML4Set = true;

    ConsolePrint(u"Now in long mode!...\n");
}
