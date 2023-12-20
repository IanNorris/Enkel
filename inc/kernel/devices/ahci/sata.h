#pragma once

class SataBus
{
public:
	bool Initialize(EFI_DEV_PATH* devicePath);

	volatile uint8_t* GetCommandListBase(uint32_t portNumber);

	void ResetPort(uint32_t portNumber);

	void StartCommandEngine(uint32_t portNumber);
	void StopCommandEngine(uint32_t portNumber);

public:

	ACPI_PCI_ID PciId;
	volatile HBAMemory* Memory;

	uint8_t* CommandList;
};
