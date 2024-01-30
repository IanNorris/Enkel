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

#include "fs/volume.h"

//#define VerboseSataLog(x) SerialPrint(x)
#define VerboseSataLog(x)

#define TO_LOW_HIGH(low, high, input) low = (uint32_t)((uint64_t)input & 0xffffffff); high = (uint32_t)(((uint64_t)input >> 32) & 0xffffffff);

// Forward declaration of AHCI related functions

extern "C" volatile UINT64* PciConfigGetMMIOAddress(UINT8 bus, UINT8 device, UINT8 function, UINT8 offset);
extern "C" ACPI_STATUS AcpiOsReadPciConfiguration(ACPI_PCI_ID *PciId, UINT32 Register, UINT64 *Value, UINT32 Width);

///////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_CDROM_DEVICES 64

CdRomDevice* CdromDevices[MAX_CDROM_DEVICES] = {};
uint64_t CdromDeviceCount = 0;

//TODO: BAAAAAAAAAD
uint64_t GTempCDRomBufferSize;
uint8_t* GTempCDRomBuffer;


VolumeOpenHandleType CdromVolume_OpenHandle = 
[](VolumeFileHandle volumeHandle, void* context, const char16_t* path, uint8_t mode)
{
	return volumeHandle;
};

VolumeCloseHandleType CdromVolume_CloseHandle = 
[](VolumeFileHandle handle, void* context)
{};

VolumeReadType CdromVolume_Read = 
[](VolumeFileHandle handle, void* context, uint64_t offset, void* buffer, uint64_t size) -> uint64_t
{
	CdRomDevice* cdrom = (CdRomDevice*)context;

	const uint64_t sectorSize = 2048;

	uint64_t startSector = (offset & ~(sectorSize-1)) / sectorSize;
	uint64_t sectorCount = (AlignSize(offset, sectorSize) / sectorSize) - startSector;
	uint64_t sectorCountUpperBound = 1 + sectorCount;
	uint64_t sectorUpperBoundSize = sectorCountUpperBound * sectorSize;

	if(GTempCDRomBufferSize < sectorUpperBoundSize)
	{
		if(GTempCDRomBuffer)
		{
			rpfree(GTempCDRomBuffer);
		}

		GTempCDRomBufferSize = sectorUpperBoundSize;
		GTempCDRomBuffer = (uint8_t*)rpmalloc(sectorUpperBoundSize);
	}

	uint64_t readSize = cdrom->ReadSectors(startSector, sectorCountUpperBound, GTempCDRomBuffer);

	uint64_t writeSize = min(readSize, size);

	memcpy(buffer, GTempCDRomBuffer + offset - (startSector*sectorSize), writeSize);
	
	return writeSize;
};

Volume CdromVolume
{
	OpenHandle: CdromVolume_OpenHandle,
	CloseHandle: CdromVolume_CloseHandle,
	Read: CdromVolume_Read,
	Write: nullptr,
	GetSize: nullptr
};

///////////////////////////////////////////////////////////////////////////////////////////////

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
				VerboseSataLog(u"SATA boot device found.\n");
			}
		}
		else if(devicePath->DevPath.Type == MEDIA_DEVICE_PATH)
		{
			if(devicePath->DevPath.SubType == MEDIA_CDROM_DP)
			{
				cdromDevicePath = (CDROM_DEVICE_PATH*)devicePath;
				VerboseSataLog(u"CD-ROM media.\n");
			}
			else if(devicePath->DevPath.SubType == MEDIA_HARDDRIVE_DP)
			{
				hddDevicePath = (HARDDRIVE_DEVICE_PATH*)devicePath;
				VerboseSataLog(u"HDD media.\n");
			}
			else if(devicePath->DevPath.SubType == MSG_USB_DP)
			{
				usbDevicePath = (USB_DEVICE_PATH*)devicePath;
				VerboseSataLog(u"USB media.\n");
			}
			else if(devicePath->DevPath.SubType == MSG_USB_CLASS_DP)
			{
				usbClassDevicePath = (USB_CLASS_DEVICE_PATH*)devicePath;
				VerboseSataLog(u"USB class media.\n");
			}
			else if(devicePath->DevPath.SubType == MSG_USB_WWID_DP)
			{
				usbWwidDevicePath = (USB_WWID_DEVICE_PATH*)devicePath;
				VerboseSataLog(u"USB WWID media.\n");
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

	int volumeId = CdromDeviceCount;

	//Build the volume root: /device/cdrom/0 etc
	char16_t volumeRoot[64];
	char16_t volumeIdStr[16];
	strcat(volumeRoot, u"/device/cdrom/");

	witoabuf(volumeIdStr, (int)volumeId, 10);
	strcat(volumeRoot, volumeIdStr);

	VolumeId = MountVolume(&CdromVolume, volumeRoot, this);

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

uint64_t CdRomDevice::ReadSectors(uint64_t lba, uint64_t sectorCount, uint8_t* buffer)
{
	if(CdPort->IsATAPI())
	{
		return CdPort->ReadSectorsCD(lba, sectorCount, buffer);
	}
	else
	{
		return CdPort->ReadSectorsSATA(lba, sectorCount, buffer);
	}
}

uint64_t CdRomDevice::ReadToc(uint8_t* buffer, uint32_t bufferSize)
{
	if(CdPort->IsATAPI())
	{
		return CdPort->ReadToc(buffer, bufferSize);
	}
	else
	{
		NOT_IMPLEMENTED()
		return 0;
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
	uint64_t CommandRegisterValue = port->CommandAndStatus;

	CommandRegisterValue &= ~CommandMask;
	CommandRegisterValue |= PCI_COMMAND_INTERRUPT_DISABLE | PCI_COMMAND_BUS_MASTER | PCI_COMMAND_MEMORY_SPACE | PCI_COMMAND_IO_SPACE;

	// Configure memory space for command list (this is allocated by the SataBus)
	volatile uint8_t* portCommandListAddress = sataBus->GetCommandListBase(portNumber);
	CommandLists = (HBACommandListHeader*)portCommandListAddress;

	port->CommandAndStatus = CommandRegisterValue;
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

	if(IsATAPI())
	{
		port->CommandAndStatus |= HBA_PxCMD_ATAPI;
	}

	// Enable interrupts for the port
	//port->InterruptEnable |= HBA_PxIE_DHR;

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

void CdromPort::ConvertToString(char* dest, const uint16_t* src, size_t wordCount)
{
    for (size_t i = 0; i < wordCount; ++i)
	{
        dest[i * 2] = src[i] >> 8;
        dest[i * 2 + 1] = src[i] & 0xff;
    }
    dest[wordCount * 2] = '\0';
}

bool CdromPort::IdentifyDrive()
{
    // Prepare command header and command table

	while (Port->SATAStatus & STS_BSY) // While the device is busy
	{
		// Wait
	}

    // Prepare the IDENTIFY command (ATA Command)
    uint8_t* identifyBuffer = (uint8_t*)VirtualAlloc(4096, PrivilegeLevel::Kernel, PageFlags_Cache_WriteThrough);
    memset(identifyBuffer, 0, 512);

	uint64_t identifyBufferPhysical = GetPhysicalAddress((uint64_t)identifyBuffer);

	uint32_t commandIndex = 0;

    volatile FISRegisterHostToDevice* cmdFis = (volatile FISRegisterHostToDevice*)&(CommandTables[commandIndex]->CommandFIS[0]);
    memset(cmdFis, 0, sizeof(FISRegisterHostToDevice));
    cmdFis->FISType = (uint8_t)FISType::RegisterHostToDevice;

	//GOTCHA here! The identify command is different for ATAPI devices
    cmdFis->Command = IsATAPI() ? ATA_CMD_IDENTIFY_ATAPI : ATA_CMD_IDENTIFY;
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

	const uint32_t BusyMask = STS_BSY | STS_DRQ;
	while (((Port->CommandIssue & (1 << commandIndex)) || (Port->InterruptStatus & BusyMask)))
	{
		if (Port->InterruptStatus & (HBA_PxIS_TFES | IS_ERR_FATAL))
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

	if(Port->TaskFileData & STS_ERR)
	{
		ConsolePrintNumeric(u"Command failed: ", Port->TaskFileData >> 8 & 0xFF, u"\n", 16);
		return false;
	}

	// Check if the command completed successfully
	if (Port->CommandAndStatus & HBA_PxCMD_CR)
	{
		//const uint64_t LBA48AddressableSectors = *(uint32_t*)(identifyBuffer+ATA_IDENTIFY_MAX_LBA_EXT);
		const uint16_t* SerialNumberPtr = (uint16_t*)(identifyBuffer+ATA_IDENTIFY_SERIAL);
		const uint16_t* ModelNumberPtr = (uint16_t*)(identifyBuffer+ATA_IDENTIFY_MODEL);
		const uint16_t* FirmwareRevPtr = (uint16_t*)(identifyBuffer+ATA_IDENTIFY_FIRMWARE_REV);
		//const uint16_t BytesPerSector = *(uint16_t*)(identifyBuffer+ATA_IDENTIFY_BYTES_PER_SECTOR);

		char modelName[41];
		char serial[21];
		char firmware[9];
		ConvertToString(modelName, ModelNumberPtr, 20);
		ConvertToString(serial, SerialNumberPtr, 10);
		ConvertToString(firmware, FirmwareRevPtr, 4);

		VerboseSataLog(u"Drive model: ");
		VerboseSataLog(modelName);

		VerboseSataLog(u"\nSerial: ");
		VerboseSataLog(serial);

		VerboseSataLog(u"\nFirmware: ");
		VerboseSataLog(firmware);

		//ConsolePrintNumeric(u"\nLBA count: ", LBA48AddressableSectors, u"\n", 10);
		//ConsolePrintNumeric(u"Sector size: ", BytesPerSector, u"\n", 10);
		//ConsolePrintNumeric(u"Byte size: ", BytesPerSector * LBA48AddressableSectors, u"\n", 10);
	}
	else
	{
		// Command failed
		SerialPrint("IDENTIFY command failed.\n");
		return false;
	}

	while (Port->SATAStatus & STS_BSY) // While the device is busy
	{
		// Wait
	}

	// Free allocated buffer
	VirtualFree(identifyBuffer, 4096);

    return true;
}

uint64_t CdromPort::ReadSectorsSATA(uint64_t lba, uint64_t sectorCount, uint8_t* buffer)
{
	uint32_t sectorSize = 2048;
	uint32_t byteSize = sectorCount * sectorSize;

	_ASSERTF(byteSize <= 4 * 1024 * 1024, "PRDT limited to 4MB each.")

	while (Port->SATAStatus & STS_BSY) // While the device is busy
	{
		// Wait
	}

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

	//TO_LOW_HIGH(cmdFis->CountLow, cmdFis->CountHigh, (byteSize / sectorSize));

    cmdFis->CommandControl = 1; // Command

    // Set up the PRDT
    TO_LOW_HIGH(CommandTables[commandIndex]->PRDTEntries[0].DataBaseLow, CommandTables[commandIndex]->PRDTEntries[0].DataBaseHigh, bufferPhysical);
    CommandTables[commandIndex]->PRDTEntries[0].ByteCount = byteSize-1;
    CommandTables[commandIndex]->PRDTEntries[0].InterruptOnCompletion = 1;

    // Set command header
	CommandLists[commandIndex].Atapi = 0;
    CommandLists[commandIndex].PRDTLength = 1;
    CommandLists[commandIndex].CommandFISLength = sizeof(FISRegisterHostToDevice) / sizeof(uint32_t); // Command FIS size in DWORDs
    CommandLists[commandIndex].Write = 0; // This is a read
    CommandLists[commandIndex].PRDByteCount = 0; // Clear, this is set by the hardware

    // Start command and wait for completion
    StartCommand(commandIndex);

    // Wait for command to complete
    const uint32_t BusyMask = STS_BSY | STS_DRQ;
	while (((Port->CommandIssue & (1 << commandIndex)) || (Port->InterruptStatus & BusyMask)))
    {
        if (Port->InterruptStatus & HBA_PxIS_TFES)
        {
            // Task File Error Status
            SerialPrint("READ SECTOR command failed.\n");
            return false;
        }
    }

	while (Port->SATAStatus & STS_BSY) // While the device is busy
	{
		// Wait
	}

    if (Port->SATAError)
    {
        _ASSERTFV(false, "SATA Error", Port->SATAError, 0, 0);
    }

	uint8_t ErrorCode = (Port->TaskFileData >> 8) & 0xFF;
	if(Port->TaskFileData & STS_ERR || ErrorCode != 0)
	{
		ConsolePrintNumeric(u"Command failed: ", Port->TaskFileData >> 8 & 0xFF, u"\n", 16);
		return false;
	}

    return CommandLists[commandIndex].PRDByteCount;
}

uint64_t CdromPort::ReadSectorsCD(uint64_t lba, uint64_t sectorCount, uint8_t* buffer)
{
	uint32_t sectorSize = 2048;
	uint32_t byteSize = sectorCount * sectorSize;

	_ASSERTF(sectorCount < 256, "READ(12) limited to 255 sectors.");
	_ASSERTF(byteSize <= 4 * 1024 * 1024, "PRDT limited to 4MB each.");

	uint32_t prdtCount = 1;

	while (Port->SATAStatus & STS_BSY) // While the device is busy
	{
		// Wait
	}

	uint64_t bufferPhysical = GetPhysicalAddress((uint64_t)buffer);

	uint32_t commandIndex = 1; // Assuming we are using the first command slot

	// Prepare the command FIS for ATAPI command
	volatile FISRegisterHostToDevice* cmdFis = (volatile FISRegisterHostToDevice*)&(CommandTables[commandIndex]->CommandFIS[0]);
	memset(cmdFis, 0x0, sizeof(FISRegisterHostToDevice));
	cmdFis->FISType = (uint8_t)FISType::RegisterHostToDevice;
	cmdFis->FeatureLow = 1; // Set feature to 1 - DMA
	cmdFis->Command = ATA_CMD_PACKET; // ATAPI Packet command
	cmdFis->CommandControl = 1; // Command

	// Prepare the ATAPI command (READ (10))
	volatile uint8_t* atapiCommand = &CommandTables[commandIndex]->AtapiCommand[0];
	memset(atapiCommand, 0x0, 12); // ATAPI commands are 12 bytes
	atapiCommand[0] = ATAPI_CMD_READ_12;
	atapiCommand[1] = 0;
	atapiCommand[2] = (lba >> 24) & 0xFF;
	atapiCommand[3] = (lba >> 16) & 0xFF;
	atapiCommand[4] = (lba >> 8) & 0xFF;
	atapiCommand[5] = lba & 0xFF;

	atapiCommand[9] = sectorCount == 256 ? 0 : sectorCount & 0xFF;
	
	// Set up the PRDT
	memset(&CommandTables[commandIndex]->PRDTEntries[0], 0, sizeof(HBAPRDTEntry) * prdtCount);
	TO_LOW_HIGH(CommandTables[commandIndex]->PRDTEntries[0].DataBaseLow, CommandTables[commandIndex]->PRDTEntries[0].DataBaseHigh, bufferPhysical);
	CommandTables[commandIndex]->PRDTEntries[0].ByteCount = byteSize-1;
	CommandTables[commandIndex]->PRDTEntries[0].InterruptOnCompletion = 0;

	// Set command header
	CommandLists[commandIndex].Atapi = 1;
	CommandLists[commandIndex].PRDTLength = prdtCount;
	CommandLists[commandIndex].CommandFISLength = sizeof(FISRegisterHostToDevice) / sizeof(uint32_t); // Command FIS size in DWORDs
	CommandLists[commandIndex].Write = 0; // This is a read
	CommandLists[commandIndex].PRDByteCount = 0; // Clear, this is set by the hardware

	// Start command and wait for completion
	StartCommand(commandIndex);

	// Wait for command to complete
	const uint32_t BusyMask = STS_BSY | STS_DRQ;
	while (((Port->CommandIssue & (1 << commandIndex)) || (Port->InterruptStatus & BusyMask)))
	{
		if (Port->InterruptStatus & HBA_PxIS_TFES)
		{
			// Task File Error Status
			SerialPrint("READ CD SECTOR command failed.\n");
			return 0; // Return 0 bytes read on failure
		}
	}

	uint8_t ErrorCode = (Port->TaskFileData >> 8) & 0xFF;
	if(Port->TaskFileData & STS_ERR || ErrorCode != 0)
	{
		ConsolePrintNumeric(u"Command failed: ", Port->TaskFileData >> 8 & 0xFF, u"\n", 16);
		return 0; // Return 0 bytes read on failure
	}

	if (Port->SATAError)
	{
		_ASSERTFV(false, "SATA Error", Port->SATAError, 0, 0);
	}

	return CommandLists[commandIndex].PRDByteCount;
}

uint64_t CdromPort::ReadToc(uint8_t* buffer, uint32_t bufferSize)
{
    _ASSERTF(buffer != nullptr, "Buffer cannot be null");

	uint32_t startTrack = 0;
	uint8_t format = 0;

    uint32_t prdtCount = 1;

	while (Port->SATAStatus & STS_BSY) // While the device is busy
	{
		// Wait
	}

    // Ensure buffer is allocated and cleared
    memset(buffer, 0, bufferSize);

    uint64_t bufferPhysical = GetPhysicalAddress((uint64_t)buffer);
    uint32_t commandIndex = 0; // Using the first command slot

    // Prepare the command FIS for ATAPI command
    volatile FISRegisterHostToDevice* cmdFis = (volatile FISRegisterHostToDevice*)&(CommandTables[commandIndex]->CommandFIS[0]);
    memset(cmdFis, 0, sizeof(FISRegisterHostToDevice));
    cmdFis->FISType = (uint8_t)FISType::RegisterHostToDevice;
	cmdFis->FeatureLow = 1; // Set feature to 1 - DMA
    cmdFis->Command = ATA_CMD_PACKET;
    cmdFis->CommandControl = 1;

    // Prepare the ATAPI command (READ TOC)
    volatile uint8_t* atapiCommand = &CommandTables[commandIndex]->AtapiCommand[0];
    memset(atapiCommand, 0, 12);
    atapiCommand[0] = 0x43; // READ TOC command
    atapiCommand[2] = format; // Format
    atapiCommand[6] = startTrack; // Starting track
    atapiCommand[7] = (bufferSize >> 8) & 0xFF; // Allocation length (MSB)
    atapiCommand[8] = bufferSize & 0xFF; // Allocation length (LSB)

    // Set up the PRDT
    memset(&CommandTables[commandIndex]->PRDTEntries[0], 0, sizeof(HBAPRDTEntry) * prdtCount);
    TO_LOW_HIGH(CommandTables[commandIndex]->PRDTEntries[0].DataBaseLow, CommandTables[commandIndex]->PRDTEntries[0].DataBaseHigh, bufferPhysical);
    CommandTables[commandIndex]->PRDTEntries[0].ByteCount = bufferSize - 1;
    CommandTables[commandIndex]->PRDTEntries[0].InterruptOnCompletion = 0;

    // Set command header
    CommandLists[commandIndex].Atapi = 1;
    CommandLists[commandIndex].PRDTLength = prdtCount;
    CommandLists[commandIndex].CommandFISLength = sizeof(FISRegisterHostToDevice) / sizeof(uint32_t);
    CommandLists[commandIndex].Write = 0;
    CommandLists[commandIndex].PRDByteCount = 0;

    // Start command and wait for completion
    StartCommand(commandIndex);

    // Wait for command to complete
    const uint32_t BusyMask = STS_BSY | STS_DRQ;
    while (((Port->CommandIssue & (1 << commandIndex)) || (Port->InterruptStatus & BusyMask)))
    {
        if (Port->InterruptStatus & HBA_PxIS_TFES)
        {
            SerialPrint("READ TOC command failed.\n");
            return false;
        }
    }

	while (Port->SATAStatus & STS_BSY) // While the device is busy
	{
		// Wait
	}

    uint8_t ErrorCode = (Port->TaskFileData >> 8) & 0xFF;
    if(Port->TaskFileData & STS_ERR || ErrorCode != 0)
    {
        ConsolePrintNumeric(u"TOC command failed: ", Port->TaskFileData >> 8 & 0xFF, u"\n", 16);
        return false;
    }

    if (Port->SATAError)
    {
        _ASSERTFV(false, "SATA Error", Port->SATAError, 0, 0);
    }

    return CommandLists[commandIndex].PRDByteCount;
}


void CdromPort::StartCommand(uint32_t slot)
{
	//Force it to update
	Port->InterruptStatus = Port->InterruptStatus;

    Port->CommandIssue |= (1 << slot);
}

void CdromPort::LogStatus()
{
	SerialPrint(u"Device state: ");

	if(Port->CommandAndStatus & HBA_PxCMD_ST)
	{
		SerialPrint(u"ST ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_SUD)
	{
		SerialPrint(u"SUD ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_POD)
	{
		SerialPrint(u"POD ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_CLO)
	{
		SerialPrint(u"CLO ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_FRE)
	{
		SerialPrint(u"FRE ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_MPSS)
	{
		SerialPrint(u"MPSS ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_FR)
	{
		SerialPrint(u"FR ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_CR)
	{
		SerialPrint(u"CR ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_CPS)
	{
		SerialPrint(u"CPS ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_PMA)
	{
		SerialPrint(u"PMA ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_HPCP)
	{
		SerialPrint(u"HPCP ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_MPSP)
	{
		SerialPrint(u"MPSP ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_CPD)
	{
		SerialPrint(u"CPD ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_ESP)
	{
		SerialPrint(u"ESP ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_FBSCP)
	{
		SerialPrint(u"FBSCP ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_APSTE)
	{
		SerialPrint(u"APSTE ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_ATAPI)
	{
		SerialPrint(u"ATAPI ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_DLAE)
	{
		SerialPrint(u"DLAE ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_ALPE)
	{
		SerialPrint(u"ALPE ");
	}

	if(Port->CommandAndStatus & HBA_PxCMD_ASP)
	{
		SerialPrint(u"ASP ");
	}

	SerialPrint(u"\n");
}
