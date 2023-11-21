#pragma once

class SataBus
{
public:
	bool Initialize(EFI_DEV_PATH* devicePath);

public:

	ACPI_PCI_ID PciId;
	volatile HBAMemory* Memory;

	uint8_t* CommandList;
};
