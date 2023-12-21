#pragma once

#include "kernel/devices/ahci/ahci.h"
#include "kernel/devices/ahci/sata.h"

class CdromPort
{
public:

	bool Initialize(SataBus* sataBus, uint32_t portNumber);

	uint64_t ReadSectorSATA(uint64_t lba, uint8_t* buffer);
	uint64_t ReadSectorCD(uint64_t lba, uint8_t* buffer);

	bool IsActive();
	bool IsATAPI();

private:

	bool IdentifyDrive();
	void StartCommand(uint32_t slot);

	const static uint32_t MaxPRDTEntries = 16; //Up to 64k count at 4MB each.
	const static uint32_t MaxCommands = 32;
	const static uint32_t PRDTSize = 64 * 1024;

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

	uint64_t ReadSector(uint64_t lba, uint8_t* buffer);

private:

	const static uint32_t MaxPorts = 32;

	void SetupPorts();

	SataBus* Sata;
	CdromPort Ports[MaxPorts];
	CdromPort* CdPort;

	ACPI_PCI_ID PciId;	
};
