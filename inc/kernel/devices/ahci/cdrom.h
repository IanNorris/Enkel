#pragma once

#include "kernel/devices/ahci/ahci.h"
#include "kernel/devices/ahci/sata.h"
#include "fs/volume.h"

class CdromPort
{
public:

	bool Initialize(SataBus* sataBus, uint32_t portNumber);

	uint64_t ReadToc(uint8_t* buffer, uint32_t bufferSize);
	uint64_t ReadSectorsSATA(uint64_t lba, uint64_t sectorCount, uint8_t* buffer);
	uint64_t ReadSectorsCD(uint64_t lba, uint64_t sectorCount, uint8_t* buffer);

	bool IsActive();
	bool IsATAPI();

	void LogStatus();

private:

	bool IdentifyDrive();
	void StartCommand(uint32_t slot);

	void ConvertToString(char* dest, const uint16_t* src, size_t wordCount);

	const static uint32_t MaxPRDTEntries = 16; //Up to 64k count at 4MB each.
	const static uint32_t MaxCommands = 32;

	volatile HBAPort* Port;
	volatile HBACommandListHeader* CommandLists;
	volatile uint8_t* CommandTableAlloc;

	volatile FISData* FIS;
	volatile HBACommandTable* CommandTables[MaxCommands];

	SataBus* Sata;
	uint32_t PortNumber;
	bool IsEnabled;
};

class CdRomDevice
{
public:
	bool Initialize(EFI_DEV_PATH* devicePath, SataBus* sataBus);

	uint64_t ReadSectors(uint64_t lba, uint64_t sectorCount, uint8_t* buffer);
	uint64_t ReadToc(uint8_t* buffer, uint32_t bufferSize);

	VolumeHandle GetVolumeId() { return VolumeId; }

private:

	const static uint32_t MaxPorts = 32;

	void SetupPorts();

	SataBus* Sata;
	CdromPort Ports[MaxPorts];
	CdromPort* CdPort;

	ACPI_PCI_ID PciId;	

	VolumeHandle VolumeId;
};

