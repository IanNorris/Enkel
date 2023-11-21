#pragma once

extern "C"
{
	#include <acpi.h>
}

#include "Uefi.h"
#include <Protocol/DevicePath.h>

ACPI_MODULE_NAME("enkel")

#define ACPI_PCI_BUS(adr)      ((uint8_t)((adr) >> 16))
#define ACPI_PCI_DEVICE(adr)   ((uint8_t)(((adr) >> 3) & 0x1F))
#define ACPI_PCI_FUNCTION(adr) ((uint8_t)((adr) & 0x07))

ACPI_STATUS InitializeAcpica (void);
void WalkAcpiTree();
bool FindSpecificDevice(uint32_t hid, uint32_t uid, ACPI_HANDLE* outDevice);
ACPI_STATUS GetPciIdFromAcpiHandle(ACPI_HANDLE Handle, ACPI_PCI_ID* PciId);

unsigned int DevicePathNodeLength(const EFI_DEVICE_PATH_PROTOCOL *DevicePath);
EFI_DEVICE_PATH_PROTOCOL* NextDevicePathNode(EFI_DEVICE_PATH_PROTOCOL *DevicePath);
