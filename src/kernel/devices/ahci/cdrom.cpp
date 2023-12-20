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
#define MAX_PRDT_ENTRIES 65536

//TODO: Verify
#define HBA_PxIE_DHR 0x80000000 // Device to host register FIS interrupt enable
#define HBA_PxIS_TFES 0x40000000 // Task file error status
#define ATA_CMD_IDENTIFY 0xEC

#define PCI_BAR5_OFFSET 0x24

#define PCI_COMMAND_INTERRUPT_DISABLE (1 << 10)
#define PCI_COMMAND_BUS_MASTER (1 << 2)
#define PCI_COMMAND_MEMORY_SPACE (1 << 1)
#define PCI_COMMAND_IO_SPACE (1 << 0)

#define TO_LOW_HIGH(low, high, input) low = (uint32_t)((uint64_t)input & 0xffffffff); high = (uint32_t)(((uint64_t)input >> 32) & 0xffffffff);

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

	UNUSED(cdromDevicePath);
	UNUSED(hddDevicePath);
	UNUSED(usbDevicePath);
	UNUSED(usbClassDevicePath);
	UNUSED(usbWwidDevicePath);

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

	SetupPorts();

	CdPort = &Ports[sataDevicePath->HBAPortNumber];

	if(!CdPort->IsActive())
	{
		ConsolePrint(u"CD-ROM drive not found on expected SATA port.\n");
		return false;
	}

	return true;
}

void CdRomDevice::SetupPorts()
{
	memset(Ports, 0, sizeof(Ports[0]) * MaxPorts);

    for (uint32_t i = 0; i < MaxPorts; i++)
	{
		Ports[i].Initialize(Sata, i);
    }
}

//////////////////////////////////////////////////////////////////////////

bool CdromPort::Initialize(SataBus* sataBus, uint32_t portNumber)
{
	Sata = sataBus;
	PortNumber = portNumber;
	IsEnabled = false;

	//TODO PortMultiplier not implemented
	volatile HBAPort* port = &sataBus->Memory->Ports[portNumber];
	Port = port;

	if(port->Signature != SATA_SIG_ATAPI)
	{
		return false;
	}

	if(!(port->SATAStatus & HBA_PORT_IPM_ACTIVE))
	{
		return false;
	}

	if(!(port->SATAStatus & HBA_PORT_DET_PRESENT))
	{
		return false;
	}

	//Enable interrupts, DMA, and memory space access
	// (actually we'll disable interrupts)

	uint64_t CommandMask = PCI_COMMAND_INTERRUPT_DISABLE | PCI_COMMAND_BUS_MASTER | PCI_COMMAND_MEMORY_SPACE | PCI_COMMAND_IO_SPACE;
	uint64_t CommandRegisterValue = port->CommandStatus;

	CommandRegisterValue &= ~CommandMask;
	CommandRegisterValue |= PCI_COMMAND_INTERRUPT_DISABLE | PCI_COMMAND_BUS_MASTER | PCI_COMMAND_MEMORY_SPACE | PCI_COMMAND_IO_SPACE;

	// Configure memory space for command list (this is allocated by the SataBus)
	volatile uint8_t* portCommandListAddress = sataBus->GetCommandListBase(portNumber);
	CommandList = (HBACommandListHeader*)portCommandListAddress;

	port->CommandStatus = CommandRegisterValue;
	TO_LOW_HIGH(port->CommandListBaseLow, port->CommandListBaseHigh, portCommandListAddress);

	// The alignment constraints for FIS and command tables are:
	// FIS: 256-byte aligned
	// Command tables: 128-byte aligned

	uint64_t FISSize = AlignSize(sizeof(FIS), 128);
	
	//-1 entries because we have one inital entry in the command table
	uint64_t CommandTableAllocSize = FISSize + sizeof(HBACommandTable) + (sizeof(HBAPRDTEntry) * (MaxPRDTEntries-1));
	CommandTableAllocSize = AlignSize(CommandTableAllocSize, PAGE_SIZE);

	//TODO: Free me
	CommandTableAlloc = (volatile uint8_t*)VirtualAlloc(CommandTableAllocSize, PrivilegeLevel::Kernel, PageFlags_Cache_Disable);
	memset(CommandTableAlloc, 0, CommandTableAllocSize);

	volatile uint8_t* fis = CommandTableAlloc;
	volatile uint8_t* cmdTable = CommandTableAlloc + FISSize;

	_ASSERTF(((uint64_t)cmdTable & 0x7F) == 0, "Command table not aligned");
	CommandTable = (HBACommandTable*)cmdTable;

	// Set addresses in port registers
	TO_LOW_HIGH(CommandList->CommandTableBaseLow, CommandList->CommandTableBaseHigh, cmdTable);
	TO_LOW_HIGH(port->FISBaseLow, port->FISBaseHigh, fis);

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

	// Reset the port
	Sata->ResetPort(portNumber);

	// Start command list processing
	port->CommandStatus |= HBA_PxCMD_ST;

	// Enable interrupts for the port
	port->InterruptEnable = HBA_PxIE_DHR;

	if(!IdentifyDrive())
	{
		ConsolePrint(u"CD-ROM drive identification failed.\n");
		return false;
	}

	IsEnabled = true;

	return true;
}

bool CdromPort::IsActive()
{
	if(!IsEnabled)
	{
		return false;
	}

	if(Port->Signature != SATA_SIG_ATAPI)
	{
		return false;
	}

	if(!(Port->SATAStatus & HBA_PORT_IPM_ACTIVE))
	{
		return false;
	}

	if(!(Port->SATAStatus & HBA_PORT_DET_PRESENT))
	{
		return false;
	}

	return true;
}

bool CdromPort::IdentifyDrive()
{
    // Prepare command header and command table

    // Prepare the IDENTIFY command (ATA Command)
    uint16_t* identifyBuffer = (uint16_t*)rpmalloc(512);
    memset(identifyBuffer, 0, 512);

    volatile FISRegisterHostToDevice* cmdFis = (FISRegisterHostToDevice*)&CommandTable->CommandFIS;
    memset(cmdFis, 0, sizeof(FISRegisterHostToDevice));
    cmdFis->FISType = (uint8_t)FISType::RegisterHostToDevice;
    cmdFis->Command = ATA_CMD_IDENTIFY;
    cmdFis->Device = 0; // LBA mode
    cmdFis->CommandControl = 1; // Command

    // Set up the PRDT (Physical Region Descriptor Table)
	TO_LOW_HIGH(CommandTable->PRDTEntries[0].DataBaseLow, CommandTable->PRDTEntries[0].DataBaseHigh, identifyBuffer);
    CommandTable->PRDTEntries[0].ByteCount = 511; // 512 bytes, minus 1
    CommandTable->PRDTEntries[0].InterruptOnCompletion = 0;

    // Set command header
    CommandList->PRDTLength = 1;
    CommandList->CommandFISLength = sizeof(FISRegisterHostToDevice) / sizeof(uint32_t); // Command FIS size in DWORDs
    CommandList->Write = 0; // This is a read

    // Start command and wait for completion
	uint32_t slot = 0;
    StartCommand(slot);

    // Wait for command to complete
    while (Port->CommandIssue & (1 << slot))
	{
		if (Port->InterruptStatus & HBA_PxIS_TFES)
		{
			// Task File Error Status
			SerialPrint("IDENTIFY command failed.\n");
			return false;
		}
	}

    // Process IDENTIFY data
    //ProcessIdentifyData(identifyBuffer);

    // Free allocated buffer
    rpfree(identifyBuffer);

    return true;
}

void CdromPort::StartCommand(uint32_t slot)
{
    Port->CommandIssue |= (1 << slot);
}
