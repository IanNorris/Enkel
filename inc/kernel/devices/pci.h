#pragma once

#include "memory/memory.h"

extern "C"
{
#include "acpi.h"
}

extern "C" volatile UINT64* PciConfigGetMMIOAddress(UINT8 bus, UINT8 device, UINT8 function, UINT8 offset);

//From the ACPI OS interface

ACPI_STATUS AcpiOsWritePciConfiguration(ACPI_PCI_ID* PciId, UINT32 Register, UINT64 Value, UINT32 Width);
ACPI_STATUS AcpiOsReadPciConfiguration(ACPI_PCI_ID* PciId, UINT32 Register, UINT64 *Value, UINT32 Width);
