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

//TODO: Verify
#define HBA_PxIE_DHR 0x80000000 // Device to host register FIS interrupt enable
#define HBA_PxIS_TFES 0x40000000 // Task file error status
#define ATA_CMD_IDENTIFY 0xEC
#define ATA_CMD_PACKET 0xA0

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

uint64_t CdRomDevice::ReadSector(uint64_t lba, uint8_t* buffer)
{
	if(CdPort->IsATAPI())
	{
		return CdPort->ReadSectorCD(lba, buffer);
	}
	else
	{
		return CdPort->ReadSectorSATA(lba, buffer);
	}
}

//////////////////////////////////////////////////////////////////////////

bool CdromPort::Initialize(SataBus* sataBus, uint32_t portNumber)
{
	Sata = sataBus;
	PortNumber = portNumber;
	IsEnabled = false;

	// Ensure that the command engine is stopped before resetting
    Sata->StopCommandEngine(portNumber);

	//TODO PortMultiplier not implemented
	volatile HBAPort* port = &sataBus->Memory->Ports[portNumber];
	Port = port;

	//Enable interrupts, DMA, and memory space access
	// (actually we'll disable interrupts)

	uint64_t CommandMask = PCI_COMMAND_INTERRUPT_DISABLE | PCI_COMMAND_BUS_MASTER | PCI_COMMAND_MEMORY_SPACE | PCI_COMMAND_IO_SPACE;
	uint64_t CommandRegisterValue = port->CommandStatus;

	CommandRegisterValue &= ~CommandMask;
	CommandRegisterValue |= PCI_COMMAND_INTERRUPT_DISABLE | PCI_COMMAND_BUS_MASTER | PCI_COMMAND_MEMORY_SPACE | PCI_COMMAND_IO_SPACE;

	// Configure memory space for command list (this is allocated by the SataBus)
	volatile uint8_t* portCommandListAddress = sataBus->GetCommandListBase(portNumber);
	CommandLists = (HBACommandListHeader*)portCommandListAddress;

	port->CommandStatus = CommandRegisterValue;
	uint64_t portCommandListPhysical = GetPhysicalAddress((uint64_t)portCommandListAddress);
	TO_LOW_HIGH(port->CommandListBaseLow, port->CommandListBaseHigh, portCommandListPhysical);

	// The alignment constraints for FIS and command tables are:
	// FIS: 256-byte aligned
	// Command tables: 128-byte aligned

	uint64_t FISSize = AlignSize(sizeof(FISData), 128);
	uint64_t CommandListAndPRDTSize = AlignSize((sizeof(HBACommandTable) + sizeof(HBAPRDTEntry) * ((MaxCommands*MaxPRDTEntries)-1)), 128);
	
	//-1 entries because we have one inital entry in the command table
	uint64_t CommandTableAllocSize = FISSize + (CommandListAndPRDTSize * MaxCommands);
	CommandTableAllocSize = AlignSize(CommandTableAllocSize, PAGE_SIZE);

	//TODO: Free me
	CommandTableAlloc = (volatile uint8_t*)VirtualAlloc(CommandTableAllocSize, PrivilegeLevel::Kernel, PageFlags_Cache_Disable);
	memset(CommandTableAlloc, 0, CommandTableAllocSize);

	volatile uint8_t* fis = CommandTableAlloc;
	FIS = (volatile FISData*)fis;

	volatile uint8_t* cmdTable = (CommandTableAlloc + FISSize);

	uint64_t fisPhysical = GetPhysicalAddress((uint64_t)fis);
	TO_LOW_HIGH(port->FISBaseLow, port->FISBaseHigh, fisPhysical);

	for(uint32_t commandIndex = 0; commandIndex < MaxCommands; commandIndex++)
	{
		_ASSERTF(((uint64_t)cmdTable & 0x7F) == 0, "Command table not aligned");
		CommandTables[commandIndex] = (HBACommandTable*)cmdTable;

		uint64_t cmdTablePhysical = GetPhysicalAddress((uint64_t)cmdTable);

		// Set addresses in port registers
		TO_LOW_HIGH(CommandLists[commandIndex].CommandTableBaseLow, CommandLists[commandIndex].CommandTableBaseHigh, cmdTablePhysical);
		cmdTable += sizeof(HBACommandTable);

		CommandLists[commandIndex].PRDTLength = MaxPRDTEntries;
		CommandLists[commandIndex].PRDByteCount = 0;

		cmdTable += CommandListAndPRDTSize - sizeof(HBACommandTable);
	}

	// Reset the port
	Sata->ResetPort(portNumber);

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

	// Enable interrupts for the port
	port->InterruptEnable |= HBA_PxIE_DHR;

	if(!IdentifyDrive())
	{
		ConsolePrint(u"CD-ROM drive identification failed.\n");
		return false;
	}

	IsEnabled = true;

	return true;
}

bool CdromPort::IsATAPI()
{
	if(Port->Signature == SATA_SIG_ATAPI)
	{
		return true;
	}

	return false;
}

bool CdromPort::IsActive()
{
	if(!IsEnabled)
	{
		return false;
	}

	if(!IsATAPI())
	{
		//Don't support anything else yet
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
    uint16_t* identifyBuffer = (uint16_t*)VirtualAlloc(4096, PrivilegeLevel::Kernel, PageFlags_Cache_WriteThrough);
    memset(identifyBuffer, 0, 512);

	uint64_t identifyBufferPhysical = GetPhysicalAddress((uint64_t)identifyBuffer);

	uint32_t commandIndex = 0;

    volatile FISRegisterHostToDevice* cmdFis = (volatile FISRegisterHostToDevice*)&(CommandTables[commandIndex]->CommandFIS[0]);
    memset(cmdFis, 0, sizeof(FISRegisterHostToDevice));
    cmdFis->FISType = (uint8_t)FISType::RegisterHostToDevice;
    cmdFis->Command = ATA_CMD_IDENTIFY;
    cmdFis->Device = 0; // LBA mode
    cmdFis->CommandControl = 1; // Command

    // Set up the PRDT (Physical Region Descriptor Table)
	TO_LOW_HIGH(CommandTables[commandIndex]->PRDTEntries[0].DataBaseLow, CommandTables[commandIndex]->PRDTEntries[0].DataBaseHigh, identifyBufferPhysical);
    CommandTables[commandIndex]->PRDTEntries[0].ByteCount = 511; // 512 bytes, minus 1
    CommandTables[commandIndex]->PRDTEntries[0].InterruptOnCompletion = 0;

    // Set command header
    CommandLists[commandIndex].PRDTLength = 1;
    CommandLists[commandIndex].CommandFISLength = sizeof(FISRegisterHostToDevice) / sizeof(uint32_t); // Command FIS size in DWORDs
    CommandLists[commandIndex].Write = 0; // This is a read

	CommandLists[commandIndex].PRDByteCount = 0; //Clear, this is set by the hardware

    // Start command and wait for completion
	StartCommand(commandIndex);

	// Wait for command to complete
	while (Port->CommandIssue & (1 << commandIndex))
	{
		if (Port->InterruptStatus & HBA_PxIS_TFES)
		{
			// Task File Error Status
			SerialPrint("IDENTIFY command failed.\n");
			return false;
		}
	}

	if (Port->SATAError)
	{
		_ASSERTFV(false, "SATA Error", Port->SATAError, 0, 0);
	}

	// Check if the command completed successfully
	if (Port->CommandStatus & HBA_PxCMD_CR)
	{
		// Command completed successfully
		// Process IDENTIFY data
		//ProcessIdentifyData(identifyBuffer);
	}
	else
	{
		// Command failed
		SerialPrint("IDENTIFY command failed.\n");
		return false;
	}

	// Free allocated buffer
	VirtualFree(identifyBuffer, 4096);

    return true;
}

uint64_t CdromPort::ReadSectorSATA(uint64_t lba, uint8_t* buffer)
{
	uint32_t sectorSize = 2048;
	uint32_t byteSize = 2048;

    // Ensure buffer is allocated and cleared
    memset(buffer, 0, byteSize);

    uint64_t bufferPhysical = GetPhysicalAddress((uint64_t)buffer);

    uint32_t commandIndex = 0; // Assuming we are using the first command slot

    // Prepare the command FIS
    volatile FISRegisterHostToDevice* cmdFis = (volatile FISRegisterHostToDevice*)&(CommandTables[commandIndex]->CommandFIS[0]);
    memset(cmdFis, 0, sizeof(FISRegisterHostToDevice));
    cmdFis->FISType = (uint8_t)FISType::RegisterHostToDevice;
    cmdFis->Command = ATA_CMD_READ_DMA_EX;
    cmdFis->Device = 0x40; // LBA mode
    cmdFis->LBA0 = (uint8_t)(lba & 0xFF);
    cmdFis->LBA1 = (uint8_t)((lba >> 8) & 0xFF);
    cmdFis->LBA2 = (uint8_t)((lba >> 16) & 0xFF);
    cmdFis->LBA3 = (uint8_t)((lba >> 24) & 0xFF);
    cmdFis->LBA4 = (uint8_t)((lba >> 32) & 0xFF);
    cmdFis->LBA5 = (uint8_t)((lba >> 40) & 0xFF);

	TO_LOW_HIGH(cmdFis->CountLow, cmdFis->CountHigh, (byteSize / sectorSize));

    cmdFis->CommandControl = 1; // Command

    // Set up the PRDT
    TO_LOW_HIGH(CommandTables[commandIndex]->PRDTEntries[0].DataBaseLow, CommandTables[commandIndex]->PRDTEntries[0].DataBaseHigh, bufferPhysical);
    CommandTables[commandIndex]->PRDTEntries[0].ByteCount = byteSize-1;
    CommandTables[commandIndex]->PRDTEntries[0].InterruptOnCompletion = 1;

    // Set command header
    CommandLists[commandIndex].PRDTLength = 1;
    CommandLists[commandIndex].CommandFISLength = sizeof(FISRegisterHostToDevice) / sizeof(uint32_t); // Command FIS size in DWORDs
    CommandLists[commandIndex].Write = 0; // This is a read
    CommandLists[commandIndex].PRDByteCount = 0; // Clear, this is set by the hardware

    // Start command and wait for completion
    StartCommand(commandIndex);

    // Wait for command to complete
    while (Port->CommandIssue & (1 << commandIndex))
    {
        if (Port->InterruptStatus & HBA_PxIS_TFES)
        {
            // Task File Error Status
            SerialPrint("READ SECTOR command failed.\n");
            return false;
        }
    }

    if (Port->SATAError)
    {
        _ASSERTFV(false, "SATA Error", Port->SATAError, 0, 0);
    }

    // Check if the command completed successfully
    if (Port->CommandStatus & HBA_PxCMD_CR)
    {
        // Command completed successfully
        // Data is now in 'buffer'
		return CommandLists[commandIndex].PRDByteCount;
    }
    else
    {
        // Command failed
        SerialPrint("READ SECTOR command failed.\n");
        return 0;
    }
}

uint64_t CdromPort::ReadSectorCD(uint64_t lba, uint8_t* buffer)
{
	uint32_t sectorSize = 2048;
	uint32_t byteSize = 2048;

    // Ensure buffer is allocated and cleared
    memset(buffer, 0, byteSize);

    uint64_t bufferPhysical = GetPhysicalAddress((uint64_t)buffer);

    uint32_t commandIndex = 0; // Assuming we are using the first command slot

    // Prepare the command FIS for ATAPI command
    volatile FISRegisterHostToDevice* cmdFis = (volatile FISRegisterHostToDevice*)&(CommandTables[commandIndex]->CommandFIS[0]);
    memset(cmdFis, 0, sizeof(FISRegisterHostToDevice));
    cmdFis->FISType = (uint8_t)FISType::RegisterHostToDevice;
    cmdFis->Command = ATA_CMD_PACKET; // ATAPI Packet command
    cmdFis->CommandControl = 1; // Command

    // Prepare the ATAPI command (e.g., READ (10))
    volatile uint8_t* atapiCommand = &CommandTables[commandIndex]->AtapiCommand[0];
    memset(atapiCommand, 0, 12); // ATAPI commands are 12 bytes
    atapiCommand[0] = 0x28; // READ (10) command
    atapiCommand[2] = (lba >> 24) & 0xFF;
    atapiCommand[3] = (lba >> 16) & 0xFF;
    atapiCommand[4] = (lba >> 8) & 0xFF;
    atapiCommand[5] = lba & 0xFF;

	uint64_t sectorCount = byteSize / sectorSize;
	TO_LOW_HIGH(atapiCommand[8], atapiCommand[7], sectorCount);
    atapiCommand[7] = 0; // Number of sectors to read (high byte)
    atapiCommand[8] = 1; // Number of sectors to read (low byte)

    // Set up the PRDT
    TO_LOW_HIGH(CommandTables[commandIndex]->PRDTEntries[0].DataBaseLow, CommandTables[commandIndex]->PRDTEntries[0].DataBaseHigh, bufferPhysical);
    CommandTables[commandIndex]->PRDTEntries[0].ByteCount = byteSize-1;
    CommandTables[commandIndex]->PRDTEntries[0].InterruptOnCompletion = 1;

    // Set command header
	CommandLists[commandIndex].Atapi = 1;
    CommandLists[commandIndex].PRDTLength = 1;
    CommandLists[commandIndex].CommandFISLength = sizeof(FISRegisterHostToDevice) / sizeof(uint32_t); // Command FIS size in DWORDs
    //CommandLists[commandIndex].AtapiCommandLength = 12; // ATAPI command is 12 bytes
    CommandLists[commandIndex].Write = 0; // This is a read

    // Start command and wait for completion
    StartCommand(commandIndex);

    // Wait for command to complete
    while (Port->CommandIssue & (1 << commandIndex))
    {
        if (Port->InterruptStatus & HBA_PxIS_TFES)
        {
            // Task File Error Status
            SerialPrint("READ CD SECTOR command failed.\n");
            return false;
        }
    }

    if (Port->SATAError)
    {
        _ASSERTFV(false, "SATA Error", Port->SATAError, 0, 0);
    }

    // Check if the command completed successfully
    if (Port->CommandStatus & HBA_PxCMD_CR)
    {
        // Command completed successfully
        // Data is now in 'buffer'
    }
    else
    {
        // Command failed
        SerialPrint("READ CD SECTOR command failed.\n");
        return false;
    }
}


void CdromPort::StartCommand(uint32_t slot)
{
    Port->CommandIssue |= (1 << slot);
}
