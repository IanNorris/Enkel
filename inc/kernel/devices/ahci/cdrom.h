#pragma once

#include "kernel/devices/ahci/ahci.h"
#include "kernel/devices/ahci/sata.h"

class CdRomDevice
{
public:
	bool Initialize(EFI_DEV_PATH* devicePath, SataBus* sataBus);

private:

	SataBus* Sata;

	ACPI_PCI_ID PciId;
	volatile HBAPort* Port;

	uint8_t* CommandList;
};
