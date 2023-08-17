#include "kernel/init/init.h"
#include "kernel/init/bootload.h"
#include "kernel/init/long_mode.h"
#include "kernel/console/console.h"
#include "memory/memory.h"
#include "common/string.h"

#define PAGE_SIZE 4096
#define PAGE_MASK (~(PAGE_SIZE-1))
#define PDPT_ENTRIES 512
#define PD_ENTRIES 512
#define PT_ENTRIES (512*512)

#define PRESENT 1
#define WRITABLE 2
#define NOT_EXECUTABLE (1ULL << 63)

#define INVALID_ADDRESS (~0ULL)

struct SPagingStructurePage
{
    union
    {
        uint64_t Entries[512]; // These entries take up exactly one page
        SPagingStructurePage* Next;
    };

}__attribute__((aligned(4096)));

static_assert(EFI_PAGE_SIZE == PAGE_SIZE, "Page sizes between kernel and EFI should match.");
static_assert(sizeof(SPagingStructurePage) == PAGE_SIZE, "Paging structure should be 1 page in size.");

constexpr uint64_t PML4AddressMask = (((1ULL << 52) - 1) & ~((1ULL << 12) - 1));

#define STATIC_PAGE_ENTRIES 2048

SPagingStructurePage* PagingFreePageHead = nullptr;

// Reserve space for the page tables
SPagingStructurePage PML4 __attribute__((aligned(4096)));
SPagingStructurePage InitialPageTableEntries[STATIC_PAGE_ENTRIES] __attribute__((aligned(4096)));

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

void MapPages(uint64_t virtualAddress, uint64_t physicalAddress, uint64_t size, bool writable, bool executable, bool debug)
{
    uint64_t originalVirtualAddress = virtualAddress;
    uint64_t originalPhysicalAddress = physicalAddress;

    uint64_t endVirtualAddress = virtualAddress + size;

    virtualAddress &= PAGE_MASK;
    physicalAddress &= PAGE_MASK;

    if (debug)
    {
        ConsolePrint(u"Mapping virt 0x");
        char16_t Buffer[32];
        witoabuf(Buffer, (uint64_t)originalVirtualAddress, 16);
        ConsolePrint(Buffer);

        ConsolePrint(u"-0x");
        witoabuf(Buffer, endVirtualAddress, 16);
        ConsolePrint(Buffer);

        ConsolePrint(u" to phys 0x");
        witoabuf(Buffer, originalPhysicalAddress, 16);
        ConsolePrint(Buffer);

        ConsolePrint(u"-0x");
        witoabuf(Buffer, originalPhysicalAddress + size, 16);
        ConsolePrint(Buffer);

        ConsolePrint(u" size 0x");
        witoabuf(Buffer, size, 16);
        ConsolePrint(Buffer);

        ConsolePrint(u"\n");
    }

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

		// Set up PT
        if( PT->Entries[ptIndex] & PRESENT)
		{
            ConsolePrint(u"Existing PT entry for virt 0x");
            char16_t Buffer[32];
            witoabuf(Buffer, (uint64_t)virtualAddress, 16);
            ConsolePrint(Buffer);

            ConsolePrint(u" points to phys 0x");
            witoabuf(Buffer, PT->Entries[ptIndex] & PML4AddressMask, 16);
            ConsolePrint(Buffer);

            _ASSERTF(false, "PT already present.");
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

        uint64_t newPhysicalAddress = GetPhysicalAddress(virtualAddress);
        _ASSERTF(physicalAddress == newPhysicalAddress, "Physical address mismatch.");

		// Move to the next page
		virtualAddress += PAGE_SIZE;
		physicalAddress += PAGE_SIZE;
	}
}

void BuildPML4(const KernelBootData* bootData)
{
    PreparePML4FreeList();

    const KernelMemoryLayout& memoryLayout = bootData->MemoryLayout;

    char16_t Buffer[32];
    
    const KernelMemoryLocation& stack = bootData->MemoryLayout.SpecialLocations[SpecialMemoryLocation_KernelStack];
    const KernelMemoryLocation& binary = bootData->MemoryLayout.SpecialLocations[SpecialMemoryLocation_KernelBinary];
    const KernelMemoryLocation& framebuffer = bootData->MemoryLayout.SpecialLocations[SpecialMemoryLocation_Framebuffer];

    MapPages(stack.VirtualStart, stack.PhysicalStart, stack.ByteSize, true, true /*executable*/, false);
    MapPages(framebuffer.VirtualStart, framebuffer.PhysicalStart, framebuffer.ByteSize, true, true /*executable*/, false);
    MapPages(binary.VirtualStart, binary.PhysicalStart, binary.ByteSize, true, true /*executable*/, false);

    _ASSERTF((uint64_t)&PML4 > binary.VirtualStart && (uint64_t)&PML4 < binary.VirtualStart + binary.ByteSize, "PML4 is not inside the kernel virtual range");

    for (uint32_t entry = 0; entry < memoryLayout.Entries; entry++)
    {
        const EFI_MEMORY_DESCRIPTOR& Desc = *((EFI_MEMORY_DESCRIPTOR*)((UINT8*)memoryLayout.Map + (entry* memoryLayout.DescriptorSize)));

        if (Desc.Type != EfiConventionalMemory)
        {
            uint64_t Start = Desc.PhysicalStart;
            uint64_t End = Desc.PhysicalStart + Desc.NumberOfPages * EFI_PAGE_SIZE;
            uint64_t VirtualStart = Desc.VirtualStart;
            uint64_t VirtualEnd = VirtualStart + Desc.NumberOfPages * EFI_PAGE_SIZE;

            if (VirtualStart < 0x100000 || Start < 0x100000)
            {
                continue;
			}

            uint64_t Size = Desc.NumberOfPages * EFI_PAGE_SIZE;

            bool CareAboutPrintOut = false;
            /*if (
                    Desc.Type != EfiACPIMemoryNVS
                &&  Desc.Type != EfiBootServicesData 
                &&  Desc.Type != EfiBootServicesCode
                &&  Desc.Type != EfiRuntimeServicesCode
                &&  Desc.Type != EfiRuntimeServicesData
                &&  Desc.Type != EfiACPIReclaimMemory
                &&  Desc.Type != EfiReservedMemoryType)
            {
                CareAboutPrintOut = true;

                ConsolePrint(u"Allocated virt 0x");
                witoabuf(Buffer, VirtualStart, 16);
                ConsolePrint(Buffer);
                ConsolePrint(u"-0x");
                witoabuf(Buffer, VirtualEnd, 16);
                ConsolePrint(Buffer);

                ConsolePrint(u" to phys 0x");
                witoabuf(Buffer, Start, 16);
                ConsolePrint(Buffer);
                ConsolePrint(u"-0x");
                witoabuf(Buffer, End, 16);
                ConsolePrint(Buffer);

                witoabuf(Buffer, (uint64_t)(Desc.NumberOfPages * EFI_PAGE_SIZE), 16);
                ConsolePrint(u" size 0x");
                ConsolePrint(Buffer);
                witoabuf(Buffer, (int)Desc.Type, 10);
                ConsolePrint(u" type ");
                ConsolePrint(Buffer);
                ConsolePrint(u"\n");
            }*/

            MapPages(Desc.VirtualStart, Desc.PhysicalStart, Size, true, true /*executable*/, CareAboutPrintOut);
        }
    }
}

void BuildAndLoadPML4(const KernelBootData* bootData)
{
    BuildPML4(bootData);

    ConsolePrint(u"Loading PML4...\n");
    LoadPageMapLevel4((uint64_t)&PML4);

    ConsolePrint(u"Now in long mode!...\n");
}
