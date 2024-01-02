#include "utilities/termination.h"
#include "kernel/console/console.h"
#include "kernel/init/bootload.h"
#include "kernel/init/init.h"
#include "kernel/init/interrupts.h"
#include "memory/memory.h"
#include "memory/physical.h"
#include "rpmalloc.h"
#include "kernel/init/acpi.h"
#include "kernel/scheduling/time.h"
#include "common/string.h"

#include "kernel/devices/pci.h"
#include "kernel/devices/ahci/cdrom.h"

#define AHCI_ENABLE_FLAG 0x80000000
#define HBA_CAP2_BOH (1 << 0)
#define HBA_PORT_DET_PRESENT 0x3
#define HBA_BOHC_OS_OWNED_SEMAPHORE (1 << 0)
#define HBA_BOHC_BIOS_OWNED_SEMAPHORE (1 << 1)
#define HBA_BOHC_OS_OWNERSHIP_CHANGE_REQUEST (1 << 3)

// GHC Register Bits
#define HBA_AE_BIT (1 << 31)
#define HBA_HR_BIT (1 << 0)

#define HBA_PxCMD_SUD (1 << 1)
#define HBA_PxCMD_POD (1 << 2)
#define HBA_PxCMD_ICC_ACTIVE (1 << 28)

bool SataBus::Initialize(EFI_DEV_PATH* devicePath)
{
	memset(this, 0, sizeof(SataBus));

	ACPI_HID_DEVICE_PATH* acpiDevicePath = nullptr;
	PCI_DEVICE_PATH* pciDevicePath = nullptr;

	// Find the ACPI path
	while (devicePath->DevPath.Type != END_DEVICE_PATH_TYPE)
	{
		if(devicePath->DevPath.Type == ACPI_DEVICE_PATH)
		{
			if(devicePath->DevPath.SubType == HW_PCI_DP)
			{
				acpiDevicePath = (ACPI_HID_DEVICE_PATH*)devicePath;
				ConsolePrint(u"ACPI bus boot device.\n");
			}
		}
		else if(devicePath->DevPath.Type == HARDWARE_DEVICE_PATH)
		{
			if(devicePath->DevPath.SubType == HW_PCI_DP)
			{
				pciDevicePath = (PCI_DEVICE_PATH*)devicePath;
				ConsolePrint(u"PCI-hosted boot device.\n");
			}
		}

		devicePath = (EFI_DEV_PATH*)NextDevicePathNode( (EFI_DEVICE_PATH_PROTOCOL*)devicePath );
	}

	_ASSERTF(acpiDevicePath, "ACPI bus is mandatory");
	_ASSERTF(pciDevicePath, "PCI host is mandatory");
	
	// ACPI isn't reporting the root PCI controller, so we know the device and funcion but not the bus.

	PciId.Segment = 0; //TODO: Segment assumed to be 0
	PciId.Device = pciDevicePath->Device;
	PciId.Function = pciDevicePath->Function;

    for (uint16_t bus = 0; bus < 256; bus++)
    {
		PciId.Bus = bus;

		UINT64 classCode;
		ACPI_STATUS status = AcpiOsReadPciConfiguration(&PciId, 0x08, &classCode, 32);
		if (status == AE_OK && ((classCode >> 16) & 0xFFFF) == ((AHCI_CLASS_CODE << 8) | AHCI_SUBCLASS_CODE))
		{
			ConsolePrint(u"Valid bus found.\n");

			// Read BAR5 to get ABAR (AHCI Base Memory Register)
			UINT64 abar;
			status = AcpiOsReadPciConfiguration(&PciId, PCI_BAR5_OFFSET, &abar, 64);
			if (status != AE_OK)
			{
				_ASSERTF(false, "Failed to read ABAR");
			}

			uint64_t size = sizeof(HBAMemory) + (32 * sizeof(HBAPort));
			uint64_t alignedSize = (size + (PAGE_SIZE-1)) & PAGE_MASK;

			uint64_t physicalAddress = abar & ~0xF;

			//TODO Free this
			//Memory map BAR 5 register as uncacheable.
			Memory = (HBAMemory*)PhysicalAlloc(physicalAddress, alignedSize, PrivilegeLevel::Kernel, PageFlags_Cache_Disable);

			ConsolePrint(u"Valid bus processed.\n");

			//break;
		}
    }

	if(!(Memory->GlobalHostControl & AHCI_ENABLE_FLAG))
	{
		_ASSERTF(false, "AHCI mode boot device is mandatory");
	}

	//Perform BIOS/OS handoff if required
	if(Memory->HostCapabilitiesExtended & HBA_CAP2_BOH)
	{
		ConsolePrint(u"BIOS/OS handoff required.\n");

		Memory->BIOSHandoffControlStatus |= HBA_BOHC_OS_OWNED_SEMAPHORE;

		while(Memory->BIOSHandoffControlStatus & HBA_BOHC_BIOS_OWNED_SEMAPHORE);
	}

	ConsolePrint(u"Resetting HBA.\n");	
	Memory->GlobalHostControl |= HBA_HR_BIT;
	while(Memory->GlobalHostControl & HBA_HR_BIT);

	//Enable ACHCI
	Memory->GlobalHostControl |= HBA_AE_BIT;

	uint64_t commandListSize = ((Memory->PortsImplemented * 1024) + (PAGE_SIZE-1)) & PAGE_MASK;
	CommandList = (uint8_t*)VirtualAlloc(commandListSize, PrivilegeLevel::Kernel, PageFlags_Cache_Disable);

	//memset(CommandList, 0, commandListSize);

	return true;
}

volatile uint8_t* SataBus::GetCommandListBase(uint32_t portNumber)
{
	return CommandList + (portNumber * 1024);
}

void SataBus::StartCommandEngine(uint32_t portNumber)
{
	volatile HBAPort* port = &Memory->Ports[portNumber];

    // Set FRE (FIS Receive Enable) bit
    port->CommandAndStatus |= HBA_PxCMD_FRE;

    // Set ST (Start) bit to start the command engine
    port->CommandAndStatus |= HBA_PxCMD_ST;
}

void SataBus::StopCommandEngine(uint32_t portNumber)
{
	volatile HBAPort* port = &Memory->Ports[portNumber];

    // Clear ST (Start) bit to stop the command engine
    port->CommandAndStatus &= ~HBA_PxCMD_ST;

    // Clear FRE (FIS Receive Enable) bit
    port->CommandAndStatus &= ~HBA_PxCMD_FRE;

    // Wait until FR (FIS Receive Running) and CR (Command List Running) bits are cleared
    while (port->CommandAndStatus & (HBA_PxCMD_FR | HBA_PxCMD_CR)) {
        // Wait for the command list and FIS receive engines to stop
    }
}

void SataBus::ResetPort(uint32_t portNumber)
{
	volatile HBAPort* port = &Memory->Ports[portNumber];

	// Ensure that the command engine is stopped before resetting
    StopCommandEngine(portNumber);

	HpetSleepMS(5);

	//Write to interrupt status to clear it
	port->InterruptStatus = port->InterruptStatus;

	//Spin up device, power on device, enable interface communication control
	port->CommandAndStatus = HBA_PxCMD_ICC_ACTIVE;

	port->CommandAndStatus |= HBA_PxCMD_FRE;

	HpetSleepMS(5);

    // Reset the port
    port->CommandAndStatus |= HBA_PxCMD_CR;

	HpetSleepMS(5);

    while ((port->CommandAndStatus & HBA_PxCMD_CR && port->SATAStatus & HBA_PORT_DET_PRESENT) || port->SATAError) {
        // Wait for the command list processing to stop
		HpetSleepMS(1);
    }

	if(port->Signature == 0)
	{
		return;
	}

	//Spin up device, power on device, enable interface communication control
	port->CommandAndStatus |= HBA_PxCMD_POD | HBA_PxCMD_SUD;

	HpetSleepMS(5);

    // Reset the port
    port->CommandAndStatus |= HBA_PxCMD_CR;

	HpetSleepMS(5);

    while ((port->CommandAndStatus & HBA_PxCMD_CR && port->SATAStatus & HBA_PORT_DET_PRESENT) || port->SATAError) {
        // Wait for the command list processing to stop
		HpetSleepMS(1);
    }

	if(port->SATAError)
	{
		_ASSERTFV(false, "SATA Error", port->SATAError, 0, 0);
	}

    // Clear error status
    port->SATAError = (uint32_t)-1; // Writing 1 to each bit clears the error

    // Restart the command engine
    StartCommandEngine(portNumber);
}
