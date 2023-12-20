#pragma once

#include "kernel/devices/ahci/ahci.h"
#include "kernel/devices/ahci/sata.h"

class CdromPort
{
public:

	bool Initialize(SataBus* sataBus, uint32_t portNumber);

	bool IsActive();

private:

	bool IdentifyDrive();
	void StartCommand(uint32_t slot);

	const static uint32_t MaxPRDTEntries = 16; //Up to 64k count at 4MB each.
	const static uint32_t PRDTSize = 64 * 1024;

	volatile HBAPort* Port;
	volatile HBACommandListHeader* CommandList;
	volatile uint8_t* CommandTableAlloc;

	volatile uint8_t* FIS;
	volatile HBACommandTable* CommandTable;
	volatile HBAPRDTEntry* PRDTs;

	SataBus* Sata;
	uint32_t PortNumber;
	bool IsEnabled;
};

class CdRomDevice
{
public:
	bool Initialize(EFI_DEV_PATH* devicePath, SataBus* sataBus);

private:

	const static uint32_t MaxPorts = 32;

	void SetupPorts();

	SataBus* Sata;
	CdromPort Ports[MaxPorts];
	CdromPort* CdPort;

	ACPI_PCI_ID PciId;	
};
