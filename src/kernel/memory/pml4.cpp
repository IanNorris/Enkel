#include "kernel/init/init.h"
#include "kernel/init/bootload.h"
#include "kernel/init/long_mode.h"
#include "kernel/console/console.h"
#include "memory/memory.h"
#include "kernel/utilities/slow.h"
#include "common/string.h"

#pragma pack(push, 1)

struct PML4Entry
{
    uint64_t Present : 1;  // Page present in memory
    uint64_t ReadWrite : 1;  // Read-only if clear, read/write if set
    uint64_t UserSupervisor : 1;  // Supervisor level only if clear
    uint64_t PageWriteThrough : 1;  // Page-level write-through
    uint64_t PageCacheDisable : 1;  // Page-level cache disable
    uint64_t Accessed : 1;  // Has the page been accessed (read or written)?
    uint64_t Unused1 : 1;  // Ignored
    uint64_t Zero : 1;  // Must be zero for PML4
    uint64_t Unused2 : 1;  // Ignored
    uint64_t Available : 3;  // Available for software use, not checked by hardware
    uint64_t Address : 40; // Physical address of the 4-KByte aligned PDPT referenced by this entry
    uint64_t Unused3 : 11; // Available for software use, not checked by hardware
    uint64_t NoExecute : 1;  // If set, prevent code execution from the associated page
} __attribute__((packed));

struct PDPTEntry
{
    uint64_t Present : 1;   // Page present in memory
    uint64_t ReadWrite : 1;   // Read/write permissions
    uint64_t UserSupervisor : 1;   // Supervisor level only if set
    uint64_t PageWriteThrough : 1;   // Write-through caching enabled
    uint64_t PageCacheDisable : 1;   // Cache disabled
    uint64_t Accessed : 1;   // Has the page been accessed since last refresh?
    uint64_t Reserved1 : 2;   // Reserved bits
    uint64_t Available : 3;   // Bits available for the OS
    uint64_t PDAddress : 40;  // Page Directory Base Address (bits 12-51)
    uint64_t Available2 : 11;  // More bits available for the OS
    uint64_t NoExecute : 1;   // No execute bit
} __attribute__((packed));

struct PDEntry
{
    uint64_t Present : 1;   // Page present in memory
    uint64_t ReadWrite : 1;   // Read/write permissions
    uint64_t UserSupervisor : 1;   // Supervisor level only if set
    uint64_t PageWriteThrough : 1;   // Write-through caching enabled
    uint64_t PageCacheDisable : 1;   // Cache disabled
    uint64_t Accessed : 1;   // Has the page been accessed since last refresh?
    uint64_t Dirty : 1;   // Has the page been written to since last refresh?
    uint64_t LargePage : 1;   // Must be 1 for 2MB pages
    uint64_t Global : 1;   // If set, prevents the TLB from updating the address
    uint64_t Available : 3;   // Bits available for the OS
    uint64_t PAT : 1;   // Page attribute table index
    uint64_t Reserved1 : 8;   // Reserved bits
    uint64_t PageBaseAddress : 31;  // Page Base Address (bits 21-51)
    uint64_t Available2 : 11;  // More bits available for the OS
    uint64_t NoExecute : 1;   // No execute bit
} __attribute__((packed));


static_assert(sizeof(PML4Entry) == 8, "PML4Entry should be 8 bytes");
static_assert(sizeof(PDPTEntry) == 8, "PDPTEntry should be 8 bytes");
static_assert(sizeof(PDEntry) == 8, "PDEntry should be 8 bytes");

#pragma pack(pop)

// Reserve space for the page tables
PML4Entry PLM4Table[512] __attribute__((aligned(4096)));
PDPTEntry PDPTTable[512] __attribute__((aligned(4096)));   // For simplicity, just one PDPT for now
PDEntry   PDTable[512]   __attribute__((aligned(4096)));       // One PD, which will map 2MB pages

#define PAGE_SIZE 0x200000

#define PML4_INDEX(x) ((x >> 39) & 0x1FF)
#define PDPT_INDEX(x) ((x >> 30) & 0x1FF)
#define PD_INDEX(x)   ((x >> 21) & 0x1FF)

void Map2MBPage(uint64_t virtualAddress, uint64_t physicalAddress, bool writable, bool executable)
{
    virtualAddress &= ~(PAGE_SIZE - 1);
    physicalAddress &= ~(PAGE_SIZE - 1);

    uint64_t PML4Index = PML4_INDEX(virtualAddress);
    uint64_t PDPTIndex = PDPT_INDEX(virtualAddress);
    uint64_t PDIndex = PD_INDEX(virtualAddress);

    if (!(PLM4Table[PML4Index].Present))
    {
        PLM4Table[PML4Index].Present = true;
        PLM4Table[PML4Index].ReadWrite = writable;
        PLM4Table[PML4Index].NoExecute = !executable;
        PLM4Table[PML4Index].Address = ((uint64_t)&PDPTTable[PDPTIndex]) >> 12;
    }

    if (!(PDPTTable[PDPTIndex].Present))
    {
        PDPTTable[PDPTIndex].Present = true;
        PDPTTable[PDPTIndex].ReadWrite = writable;
        PDPTTable[PDPTIndex].NoExecute = !executable;
        PDPTTable[PDPTIndex].PDAddress = ((uint64_t)&PDTable[PDIndex]) >> 12;
    }

    PDTable[PDIndex].Present = true;
    PDTable[PDIndex].ReadWrite = writable;
    PDTable[PDIndex].NoExecute = !executable;
    PDTable[PDIndex].PageBaseAddress = physicalAddress >> 21;
}

void BuildPML4(const KernelBootData* bootData)
{
    memset(PLM4Table, 0, sizeof(PLM4Table));
    memset(PDPTTable, 0, sizeof(PDPTTable));
    memset(PDTable, 0, sizeof(PDTable));

    const KernelMemoryLayout& memoryLayout = bootData->MemoryLayout;

    ConsolePrint(u"PLM4Table located at virt 0x");

    char16_t Buffer[32];
    witoabuf(Buffer, (uint64_t)PLM4Table, 16);
    ConsolePrint(Buffer);
    ConsolePrint(u"\n");

    ConsolePrint(u"BuildPML4 located at virt 0x");
    witoabuf(Buffer, (uint64_t)&BuildPML4, 16);
    ConsolePrint(Buffer);
    ConsolePrint(u"\n");

    ConsolePrint(u"Stack located at virt 0x");
    witoabuf(Buffer, (uint64_t)Buffer, 16);
    ConsolePrint(Buffer);
    ConsolePrint(u"\n");

    Map2MBPage((uint64_t)Buffer, (uint64_t)Buffer, true, true /*executable*/);

    for (uint32_t entry = 0; entry < memoryLayout.Entries; entry++)
    {
        const EFI_MEMORY_DESCRIPTOR& Desc = *((EFI_MEMORY_DESCRIPTOR*)((UINT8*)memoryLayout.Map + (entry* memoryLayout.DescriptorSize)));

        if (Desc.Type != EfiConventionalMemory)
        {
            uint64_t Start = Desc.PhysicalStart;
            uint64_t End = Desc.PhysicalStart + Desc.NumberOfPages * EFI_PAGE_SIZE;
            uint64_t VirtualStart = Desc.VirtualStart;
            uint64_t VirtualEnd = VirtualStart + Desc.NumberOfPages * EFI_PAGE_SIZE;

            if (
                    Desc.Type != EfiACPIMemoryNVS 
                &&  Desc.Type != EfiBootServicesData 
                &&  Desc.Type != EfiBootServicesCode
                &&  Desc.Type != EfiRuntimeServicesCode
                &&  Desc.Type != EfiRuntimeServicesData
                &&  Desc.Type != EfiACPIReclaimMemory
                &&  Desc.Type != EfiReservedMemoryType)
            {
                ConsolePrint(u"Mapped virt 0x");
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
            }

            // Map each 2MB chunk
            for (uint64_t Address = Start; Address < End; Address += PAGE_SIZE)
            {
                Map2MBPage(VirtualStart, Address, true, true /*executable*/);
                VirtualStart += PAGE_SIZE;
            }

            if ((uint64_t)PLM4Table >= VirtualStart && (uint64_t)PLM4Table <= VirtualEnd)
            {
                ConsolePrint(u"PLM4Table located at virt 0x");

                witoabuf(Buffer, (uint64_t)PLM4Table, 16);
                ConsolePrint(Buffer);
                ConsolePrint(u" and phys 0x");
                witoabuf(Buffer, Start, 16);
                ConsolePrint(Buffer);
                ConsolePrint(u"\n");
            }

            if ((uint64_t)&BuildPML4 >= VirtualStart && (uint64_t)&BuildPML4 <= VirtualEnd)
            {
                ConsolePrint(u"BuildPML4 located at virt 0x");

                witoabuf(Buffer, (uint64_t)&BuildPML4, 16);
                ConsolePrint(Buffer);
                ConsolePrint(u" and phys 0x");
                witoabuf(Buffer, Start, 16);
                ConsolePrint(Buffer);
                ConsolePrint(u"\n");
            }
        }
    }

    ConsolePrint(u"Finished BuildPML4\n");

    Slow(Loitering);
}

void BuildAndLoadPML4(const KernelBootData* bootData)
{
    ConsolePrint(u"Building PML4...\n");
    Slow(Loitering);
    BuildPML4(bootData);
    Slow(Loitering);

    ConsolePrint(u"Loading PML4...\n");
    Slow(Loitering);
    LoadPageMapLevel4(PLM4Table);
    ConsolePrint(u"After PML4...\n");
    Slow(Loitering);

    ConsolePrint(u"Now in long mode!...\n");
    Slow(Loitering);
}
