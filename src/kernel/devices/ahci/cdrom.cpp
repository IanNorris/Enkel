#include "utilities/termination.h"
#include "kernel/console/console.h"
#include "kernel/init/bootload.h"
#include "kernel/init/init.h"
#include "kernel/init/interrupts.h"
#include "memory/memory.h"
#include "memory/physical.h"
#include "rpmalloc.h"
#include "kernel/init/acpi.h"
#include "common/string.h"

#include "kernel/devices/pci.h"
#include "kernel/devices/ahci/cdrom.h"

// Standard AHCI and SATA definitions
#define SATA_SIG_ATAPI 0xEB140101 // SATA signature for ATAPI devices
#define HBA_PORT_DET_PRESENT 0x3
#define HBA_PORT_IPM_ACTIVE 0x1

#define PCI_BAR5_OFFSET 0x24

#define PCI_COMMAND_INTERRUPT_DISABLE (1 << 10)
#define PCI_COMMAND_BUS_MASTER (1 << 2)
#define PCI_COMMAND_MEMORY_SPACE (1 << 1)
#define PCI_COMMAND_IO_SPACE (1 << 0)

// Forward declaration of AHCI related functions

extern "C" volatile UINT64* PciConfigGetMMIOAddress(UINT8 bus, UINT8 device, UINT8 function, UINT8 offset);
extern "C" ACPI_STATUS AcpiOsReadPciConfiguration(ACPI_PCI_ID *PciId, UINT32 Register, UINT64 *Value, UINT32 Width);

bool CdRomDevice::Initialize(EFI_DEV_PATH* devicePath, SataBus* sataBus)
{
	memset(this, 0, sizeof(CdRomDevice));

	Sata = sataBus;

	SATA_DEVICE_PATH* sataDevicePath = nullptr;
	CDROM_DEVICE_PATH* cdromDevicePath = nullptr;
	HARDDRIVE_DEVICE_PATH* hddDevicePath = nullptr;
	USB_DEVICE_PATH* usbDevicePath = nullptr;
	USB_CLASS_DEVICE_PATH* usbClassDevicePath = nullptr;
	USB_WWID_DEVICE_PATH* usbWwidDevicePath = nullptr;

	// Find the ACPI path
	while (devicePath->DevPath.Type != END_DEVICE_PATH_TYPE)
	{
		if(devicePath->DevPath.Type == MESSAGING_DEVICE_PATH)
		{
			if(devicePath->DevPath.SubType == MSG_SATA_DP)
			{
				sataDevicePath = (SATA_DEVICE_PATH*)devicePath;
				ConsolePrint(u"SATA boot device found.\n");
			}
		}
		else if(devicePath->DevPath.Type == MEDIA_DEVICE_PATH)
		{
			if(devicePath->DevPath.SubType == MEDIA_CDROM_DP)
			{
				cdromDevicePath = (CDROM_DEVICE_PATH*)devicePath;
				ConsolePrint(u"CD-ROM media.\n");
			}
			else if(devicePath->DevPath.SubType == MEDIA_HARDDRIVE_DP)
			{
				hddDevicePath = (HARDDRIVE_DEVICE_PATH*)devicePath;
				ConsolePrint(u"HDD media.\n");
			}
			else if(devicePath->DevPath.SubType == MSG_USB_DP)
			{
				usbDevicePath = (USB_DEVICE_PATH*)devicePath;
				ConsolePrint(u"USB media.\n");
			}
			else if(devicePath->DevPath.SubType == MSG_USB_CLASS_DP)
			{
				usbClassDevicePath = (USB_CLASS_DEVICE_PATH*)devicePath;
				ConsolePrint(u"USB class media.\n");
			}
			else if(devicePath->DevPath.SubType == MSG_USB_WWID_DP)
			{
				usbWwidDevicePath = (USB_WWID_DEVICE_PATH*)devicePath;
				ConsolePrint(u"USB WWID media.\n");
			}
		}

		devicePath = (EFI_DEV_PATH*)NextDevicePathNode( (EFI_DEVICE_PATH_PROTOCOL*)devicePath );
	}

	_ASSERTF(sataDevicePath, "SATA device is mandatory");

	//TODO PortMultiplier not implemented
	Port = &Sata->Memory->Ports[sataDevicePath->HBAPortNumber];

	if(Port->Signature != SATA_SIG_ATAPI)
	{
		ConsolePrint(u"SATA ATAPI signature not found.\n");
		return false;
	}

	if(!(Port->SATAStatus & HBA_PORT_IPM_ACTIVE))
	{
		ConsolePrint(u"SATA port not active.\n");
		return false;
	}

	if(!(Port->SATAStatus & HBA_PORT_DET_PRESENT))
	{
		ConsolePrint(u"SATA DET not present.\n");
		return false;
	}

	

    //Enable interrupts, DMA, and memory space access in the PCI command register
	// (actually we'll disable interrupts)

	uint64_t CommandMask = PCI_COMMAND_INTERRUPT_DISABLE | PCI_COMMAND_BUS_MASTER | PCI_COMMAND_MEMORY_SPACE | PCI_COMMAND_IO_SPACE;

	uint64_t CommandRegisterValue = Port->CommandStatus;
	//AcpiOsReadPciConfiguration(&PciId, PCI_COMMAND_REGISTER, (UINT64*)&CommandRegisterValue, 16);

	CommandRegisterValue &= ~CommandMask;
	CommandRegisterValue |= PCI_COMMAND_INTERRUPT_DISABLE | PCI_COMMAND_BUS_MASTER | PCI_COMMAND_MEMORY_SPACE | PCI_COMMAND_IO_SPACE;

	uint8_t* portCommandListAddress = Sata->CommandList + (1024 * sataDevicePath->HBAPortNumber);

	Port->CommandStatus = CommandRegisterValue;
	Port->CommandListBaseLow = (uint32_t)((uint64_t)portCommandListAddress & 0xffffffff);
	Port->CommandListBaseHigh = (uint32_t)(((uint64_t)portCommandListAddress >> 32) & 0xffffffff);

	//AcpiOsWritePciConfiguration(&PciId, PCI_COMMAND_REGISTER, CommandRegisterValue, 16);

    
    //Perform BIOS/OS handoff (if the bit in the extended capabilities is set)
    //Reset controller
    //Register IRQ handler, using interrupt line given in the PCI register. This interrupt line may be shared with other devices, so the usual implications of this apply.
    //Enable AHCI mode and interrupts in global host control register.
    //Read capabilities registers. Check 64-bit DMA is supported if you need it.
    //For all the implemented ports:
    //    Allocate physical memory for its command list, the received FIS, and its command tables. Make sure the command tables are 128 byte aligned.
    //    Memory map these as uncacheable.
    //    Set command list and received FIS address registers (and upper registers, if supported).
    //    Setup command list entries to point to the corresponding command table.
    //    Reset the port.
    //    Start command list processing with the port's command register.
    //    Enable interrupts for the port. The D2H bit will signal completed commands.
    //    Read signature/status of the port to see if it connected to a drive.
    //    Send IDENTIFY ATA command to connected drives. Get their sector size and count. 








	//Start read/write command

	//	Select an available command slot to use.
	//	Setup command FIS.
	//	Setup PRDT.
	//	Setup command list entry.
	//	Issue the command, and record separately that you have issued it. 

	//IRQ handler

	//	Check global interrupt status. Write back its value. For all the ports that have a corresponding set bit...
	//	Check the port interrupt status. Write back its value. If zero, continue to the next port.
	//	If error bit set, reset port/retry commands as necessary.
	//	Compare issued commands register to the commands you have recorded as issuing. For any bits where a command was issued but is no longer running, this means that the command has completed.
	//	Once done, continue checking if any other devices sharing the IRQ also need servicing. 
}

/*
CdRomDevice* InitializeCdRom(PCI_DEVICE_PATH* pciDevicePath, SATA_DEVICE_PATH* sataDevicePath, CDROM_DEVICE_PATH* cdromDevicePath)
{
    // Allocate memory for CdRomDevice
    CdRomDevice* device = static_cast<CdRomDevice*>(rpmalloc(sizeof(CdRomDevice)));
    if (!device)
    {
        SerialPrint(u"Failed to allocate memory for CdRomDevice\n");
        return nullptr;
    }

    // Initialize device paths
    device->PciDevicePath = pciDevicePath;
    device->SataDevicePath = sataDevicePath;
    device->CdromDevicePath = cdromDevicePath;

	// AHCI specific initialization
	HBA_MEM *ahciBase = reinterpret_cast<HBA_MEM *>(AHCI_BASE_ADDR);
	int portNumber = FindCdRomPort(ahciBase);
	if (portNumber == -1)
	{
		SerialPrint(u"No CD-ROM drive found on AHCI ports\n");
		rpfree(device);
		return nullptr;
	}

	device->Port = &ahciBase->ports[portNumber];

	// Rest of the initialization
	return device;
}

int64_t ReadBytes(CdRomDevice *device, uint64_t offset, uint8_t *buffer, uint64_t bufferSize)
{
	_ASSERTF(device != nullptr, u"Device pointer is null");
	_ASSERTF(buffer != nullptr, u"Buffer pointer is null");

	// Convert offset to LBA (Logical Block Addressing)
	uint64_t lba = offset / 2048; // Assuming 2048 bytes per sector for CD-ROM
	uint32_t sectorCount = bufferSize / 2048;

	// Read from AHCI port
	return ReadFromAHCIPort(device->Port, lba & 0xFFFFFFFF, lba >> 32, sectorCount, buffer);
}

// Implementations of AHCI related functions
int FindCdRomPort(HBA_MEM *ahciBase)
{
	// Iterate over AHCI ports to find a CD-ROM device
	for (int i = 0; i < 32; i++)
	{
		HBAPort &port = ahciBase->ports[i];
		if ((port.sig == SATA_SIG_ATAPI) && (port.ssts & HBA_PORT_IPM_ACTIVE) && (port.ssts & HBA_PORT_DET_PRESENT))
		{
			return i;
		}
	}
	return -1;
}

int ReadFromAHCIPort(HBAPort *port, uint64_t startl, uint64_t starth, uint32_t count, uint8_t *buf)
{
	// Implement AHCI read command here.
	// This will involve setting up command structures, issuing the command,
	// and waiting for completion.
	// For brevity, the details of this implementation are omitted.

	// Placeholder for actual read operation
	int64_t bytesRead = 0; // Replace with actual bytes read

	return bytesRead;
}
*/
