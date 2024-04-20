#include "memory/memory.h"
#include "kernel/init/apic.h"
#include "kernel/init/gdt.h"
#include "kernel/init/pic.h"
#include "kernel/init/msr.h"

extern "C"
{
#include "acpi.h"
#include "accommon.h"
#include "amlresrc.h"
}

extern ACPI_MCFG_ALLOCATION* GAcpiMcfgAllocation;
extern uint64_t GAciMcfgAllocationEntries;

extern "C" volatile UINT64* PciConfigGetMMIOAddress(UINT8 bus, UINT8 device, UINT8 function, UINT8 offset)
{
    // Calculate the PCI Configuration Address
    UINT64 pciOffset = static_cast<UINT64>(bus) << 20 |
                       static_cast<UINT64>(device) << 15 |
                       static_cast<UINT64>(function) << 12 |
                       (offset & 0xFF);

	ACPI_MCFG_ALLOCATION* Alloc = GAcpiMcfgAllocation;
	for (UINT64 Index = 0; Index < GAciMcfgAllocationEntries; ++Index, ++Alloc)
	{
		if(bus >= Alloc->StartBusNumber && bus <= Alloc->EndBusNumber)
		{
			volatile UINT32* configAddress = reinterpret_cast<volatile UINT32*>(Alloc->Address);
			return (volatile UINT64*)&configAddress[pciOffset >> 2];
		}
	}

	_ASSERTFV(false, "Bus not found in MMIO", bus, device, function);

	return nullptr;

    // Map the PCI configuration space to memory and read the value
    
}
