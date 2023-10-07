#pragma once

extern "C"
{
	#include <acpi.h>
}

ACPI_MODULE_NAME("enkel")

ACPI_STATUS InitializeAcpica (void);
void WalkAcpiTree();
