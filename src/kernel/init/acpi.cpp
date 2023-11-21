/******************************************************************************
 *
 * 1. Copyright Notice
 *
 * Some or all of this work - Copyright (c) 1999 - 2023, Intel Corp.
 * All rights reserved.
 *
 * 2. License
 *
 * 2.1. This is your license from Intel Corp. under its intellectual property
 * rights. You may have additional license terms from the party that provided
 * you this software, covering your right to use that party's intellectual
 * property rights.
 *
 * 2.2. Intel grants, free of charge, to any person ("Licensee") obtaining a
 * copy of the source code appearing in this file ("Covered Code") an
 * irrevocable, perpetual, worldwide license under Intel's copyrights in the
 * base code distributed originally by Intel ("Original Intel Code") to copy,
 * make derivatives, distribute, use and display any portion of the Covered
 * Code in any form, with the right to sublicense such rights; and
 *
 * 2.3. Intel grants Licensee a non-exclusive and non-transferable patent
 * license (with the right to sublicense), under only those claims of Intel
 * patents that are infringed by the Original Intel Code, to make, use, sell,
 * offer to sell, and import the Covered Code and derivative works thereof
 * solely to the minimum extent necessary to exercise the above copyright
 * license, and in no event shall the patent license extend to any additions
 * to or modifications of the Original Intel Code. No other license or right
 * is granted directly or by implication, estoppel or otherwise;
 *
 * The above copyright and patent license is granted only if the following
 * conditions are met:
 *
 * 3. Conditions
 *
 * 3.1. Redistribution of Source with Rights to Further Distribute Source.
 * Redistribution of source code of any substantial portion of the Covered
 * Code or modification with rights to further distribute source must include
 * the above Copyright Notice, the above License, this list of Conditions,
 * and the following Disclaimer and Export Compliance provision. In addition,
 * Licensee must cause all Covered Code to which Licensee contributes to
 * contain a file documenting the changes Licensee made to create that Covered
 * Code and the date of any change. Licensee must include in that file the
 * documentation of any changes made by any predecessor Licensee. Licensee
 * must include a prominent statement that the modification is derived,
 * directly or indirectly, from Original Intel Code.
 *
 * 3.2. Redistribution of Source with no Rights to Further Distribute Source.
 * Redistribution of source code of any substantial portion of the Covered
 * Code or modification without rights to further distribute source must
 * include the following Disclaimer and Export Compliance provision in the
 * documentation and/or other materials provided with distribution. In
 * addition, Licensee may not authorize further sublicense of source of any
 * portion of the Covered Code, and must include terms to the effect that the
 * license from Licensee to its licensee is limited to the intellectual
 * property embodied in the software Licensee provides to its licensee, and
 * not to intellectual property embodied in modifications its licensee may
 * make.
 *
 * 3.3. Redistribution of Executable. Redistribution in executable form of any
 * substantial portion of the Covered Code or modification must reproduce the
 * above Copyright Notice, and the following Disclaimer and Export Compliance
 * provision in the documentation and/or other materials provided with the
 * distribution.
 *
 * 3.4. Intel retains all right, title, and interest in and to the Original
 * Intel Code.
 *
 * 3.5. Neither the name Intel nor any other trademark owned or controlled by
 * Intel shall be used in advertising or otherwise to promote the sale, use or
 * other dealings in products derived from or relating to the Covered Code
 * without prior written authorization from Intel.
 *
 * 4. Disclaimer and Export Compliance
 *
 * 4.1. INTEL MAKES NO WARRANTY OF ANY KIND REGARDING ANY SOFTWARE PROVIDED
 * HERE. ANY SOFTWARE ORIGINATING FROM INTEL OR DERIVED FROM INTEL SOFTWARE
 * IS PROVIDED "AS IS," AND INTEL WILL NOT PROVIDE ANY SUPPORT, ASSISTANCE,
 * INSTALLATION, TRAINING OR OTHER SERVICES. INTEL WILL NOT PROVIDE ANY
 * UPDATES, ENHANCEMENTS OR EXTENSIONS. INTEL SPECIFICALLY DISCLAIMS ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY, NONINFRINGEMENT AND FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * 4.2. IN NO EVENT SHALL INTEL HAVE ANY LIABILITY TO LICENSEE, ITS LICENSEES
 * OR ANY OTHER THIRD PARTY, FOR ANY LOST PROFITS, LOST DATA, LOSS OF USE OR
 * COSTS OF PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, OR FOR ANY INDIRECT,
 * SPECIAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THIS AGREEMENT, UNDER ANY
 * CAUSE OF ACTION OR THEORY OF LIABILITY, AND IRRESPECTIVE OF WHETHER INTEL
 * HAS ADVANCE NOTICE OF THE POSSIBILITY OF SUCH DAMAGES. THESE LIMITATIONS
 * SHALL APPLY NOTWITHSTANDING THE FAILURE OF THE ESSENTIAL PURPOSE OF ANY
 * LIMITED REMEDY.
 *
 * 4.3. Licensee shall not export, either directly or indirectly, any of this
 * software or system incorporating such software without first obtaining any
 * required license or other approval from the U. S. Department of Commerce or
 * any other agency or department of the United States Government. In the
 * event Licensee exports any such software from the United States or
 * re-exports any such software from a foreign destination, Licensee shall
 * ensure that the distribution and export/re-export of the software is in
 * compliance with all laws, regulations, orders, or other restrictions of the
 * U.S. Export Administration Regulations. Licensee agrees that neither it nor
 * any of its subsidiaries will export/re-export any technical data, process,
 * software, or service, directly or indirectly, to any country for which the
 * United States government or any agency thereof requires an export license,
 * other governmental approval, or letter of assurance, without first obtaining
 * such license, approval or letter.
 *
 *****************************************************************************
 *
 * Alternatively, you may choose to be licensed under the terms of the
 * following license:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    substantially similar to the "NO WARRANTY" disclaimer below
 *    ("Disclaimer") and any redistribution must be conditioned upon
 *    including a substantially similar Disclaimer requirement for further
 *    binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Alternatively, you may choose to be licensed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 *****************************************************************************/

#include "utilities/termination.h"
#include "kernel/console/console.h"
#include "kernel/init/bootload.h"
#include "kernel/init/init.h"
#include "kernel/init/interrupts.h"
#include "memory/memory.h"
#include "memory/virtual.h"
#include "rpmalloc.h"
#include "kernel/init/acpi.h"
#include "common/string.h"

void NotifyHandler (
    ACPI_HANDLE                 Device,
    UINT32                      Value,
    void                        *Context)
{

    ACPI_INFO (("Received a notify 0x%X", Value));
}


ACPI_STATUS RegionInit (
    ACPI_HANDLE                 RegionHandle,
    UINT32                      Function,
    void                        *HandlerContext,
    void                        **RegionContext)
{

    if (Function == ACPI_REGION_DEACTIVATE)
    {
        *RegionContext = NULL;
    }
    else
    {
        *RegionContext = RegionHandle;
    }

    return (AE_OK);
}


ACPI_STATUS RegionHandler (
    UINT32                      Function,
    ACPI_PHYSICAL_ADDRESS       Address,
    UINT32                      BitWidth,
    UINT64                      *Value,
    void                        *HandlerContext,
    void                        *RegionContext)
{

    ACPI_INFO (("Received a region access"));

    return (AE_OK);
}

ACPI_STATUS InstallHandlers (void)
{
    ACPI_STATUS             Status;


    /* Install global notify handler */

    Status = AcpiInstallNotifyHandler (ACPI_ROOT_OBJECT,
        ACPI_SYSTEM_NOTIFY, NotifyHandler, NULL);
    if (ACPI_FAILURE (Status))
    {
        ACPI_EXCEPTION ((AE_INFO, Status, "While installing Notify handler"));
        return (Status);
    }

    Status = AcpiInstallAddressSpaceHandler (ACPI_ROOT_OBJECT,
        ACPI_ADR_SPACE_SYSTEM_MEMORY, RegionHandler, RegionInit, NULL);
    if (ACPI_FAILURE (Status))
    {
        ACPI_EXCEPTION ((AE_INFO, Status, "While installing an OpRegion handler"));
        return (Status);
    }

    return (AE_OK);
}

ACPI_MCFG_ALLOCATION* GAcpiMcfgAllocation;
uint64_t GAciMcfgAllocationEntries;

void InitializeMMIO()
{
	ACPI_TABLE_HEADER* Table;
	ACPI_STATUS Status = AcpiGetTable((char*)"MCFG", 0, &Table);

	if (ACPI_SUCCESS(Status))
	{
		// Cast to MCFG table structure
		ACPI_TABLE_MCFG* McfgTable = (ACPI_TABLE_MCFG*) Table;

		// Loop through the allocation structures
		ACPI_MCFG_ALLOCATION* Alloc = (ACPI_MCFG_ALLOCATION*) (McfgTable + 1);

		GAcpiMcfgAllocation = Alloc;
		GAciMcfgAllocationEntries = (Table->Length - sizeof(ACPI_TABLE_MCFG)) / sizeof(ACPI_MCFG_ALLOCATION);
	}
	else
	{
		// Handle error
		_ASSERTF(false, "Hardware does not support MMIO");
	}
}

ACPI_STATUS InitializeAcpica (void)
{
    ACPI_STATUS             Status;


    /* Initialize the ACPICA subsystem */

    Status = AcpiInitializeSubsystem ();
    if (ACPI_FAILURE (Status))
    {
        ACPI_EXCEPTION ((AE_INFO, Status, "While initializing ACPICA"));
        return (Status);
    }

    /* Initialize the ACPICA Table Manager and get all ACPI tables */

    ACPI_INFO (("Loading ACPI tables"));

    Status = AcpiInitializeTables (NULL, 16, FALSE);
    if (ACPI_FAILURE (Status))
    {
        ACPI_EXCEPTION ((AE_INFO, Status, "While initializing Table Manager"));
        return (Status);
    }

    /* Install local handlers */

    Status = InstallHandlers ();
    if (ACPI_FAILURE (Status))
    {
        ACPI_EXCEPTION ((AE_INFO, Status, "While installing handlers"));
        return (Status);
    }

    /* Initialize the ACPI hardware */

    Status = AcpiEnableSubsystem (ACPI_FULL_INITIALIZATION);
    if (ACPI_FAILURE (Status))
    {
        ACPI_EXCEPTION ((AE_INFO, Status, "While enabling ACPICA"));
        return (Status);
    }

    /* Create the ACPI namespace from ACPI tables */

    Status = AcpiLoadTables ();
    if (ACPI_FAILURE (Status))
    {
        ACPI_EXCEPTION ((AE_INFO, Status, "While loading ACPI tables"));
        return (Status);
    }

    /* Complete the ACPI namespace object initialization */

    Status = AcpiInitializeObjects (ACPI_FULL_INITIALIZATION);
    if (ACPI_FAILURE (Status))
    {
        ACPI_EXCEPTION ((AE_INFO, Status, "While initializing ACPICA objects"));
        return (Status);
    }

	//Now read the PCI MMIO address
	InitializeMMIO();

    return (AE_OK);
}

char GetAlphaOnly(char input)
{
	if((input >= 'A' && input <= 'Z') || (input >= 'a' && input <= 'z') || (input >= '0' && input <= '9'))
	{
		return input;
	}
	else
	{
		return input;
	}
}

void DecodeHID(uint32_t hid)
{
	if(hid & 0xFFFF == 0x41D0)
	{
		uint32_t deviceHID = (hid >> 16);
		char hidString[16] = {"PNP0"};
		witoabuf(hidString+4, deviceHID, 16);
		SerialPrint(" HIDStr: ");
		SerialPrint(hidString);

		return;
	}

    char manufacturer[4] = { 0 };
    
    manufacturer[0] = GetAlphaOnly((char)((hid >> 24) & 0xFF));
    manufacturer[1] = GetAlphaOnly((char)((hid >> 16) & 0xFF));
    manufacturer[2] = GetAlphaOnly((char)((hid >> 8) & 0xFF));
    uint8_t productID = (uint8_t)(hid & 0xFF);

	SerialPrint("Manufacturer: ");
	SerialPrint(manufacturer);

	SerialPrint(", ProductId: 0x");
	char16_t tempBuffer[16];
	witoabuf(tempBuffer, (uint32_t)productID, 16);
	SerialPrint(tempBuffer);
	SerialPrint(", ");
}


void PrintValue(ACPI_OBJECT* ObjectPtr)
{
	if (ObjectPtr->Type == ACPI_TYPE_STRING)
	{	
        SerialPrint(ObjectPtr->String.Pointer);
    } 
	else if (ObjectPtr->Type == ACPI_TYPE_INTEGER)
	{
		DecodeHID((uint32_t)ObjectPtr->Integer.Value);

        char16_t tempBuffer[16];
        witoabuf(tempBuffer, (uint32_t)ObjectPtr->Integer.Value, 16);
        SerialPrint(tempBuffer);
    }
	else
	{
        SerialPrint(" UnknownType ");
    }
	SerialPrint(", ");
}

void ReadAcpiBuffer(ACPI_HANDLE Object, const char* Name)
{
	ACPI_BUFFER buffer = {ACPI_ALLOCATE_BUFFER, NULL};

	if (ACPI_SUCCESS(AcpiEvaluateObject(Object, (ACPI_STRING)Name, NULL, &buffer)))
	{
		if(buffer.Pointer)
		{
			SerialPrint(" ");
			SerialPrint(Name);
			SerialPrint(" ");
			PrintValue((ACPI_OBJECT*)buffer.Pointer);
			ACPI_FREE(buffer.Pointer);
		}
    }
}

extern "C" const char *
AcpiUtGetTypeName (
    ACPI_OBJECT_TYPE        Type);

void PrintIndent(UINT32 depth)
{
	for(int i = 0; i < depth; i++)
	{
		SerialPrint("  ");
	}
}

ACPI_STATUS AcpiDeviceTreeCallback (
    ACPI_HANDLE                     Object,
    UINT32                          NestingLevel,
    void                            *Context,
    void                            **ReturnValue)
{
    ACPI_DEVICE_INFO* DeviceInfo;
    ACPI_STATUS Status;

	PrintIndent(NestingLevel);

	// Get the full pathname of this object
	ACPI_BUFFER NameBuffer = {ACPI_ALLOCATE_BUFFER, NULL};
    Status = AcpiGetName(Object, ACPI_FULL_PATHNAME, &NameBuffer);

	const char* Name = (const char*)NameBuffer.Pointer;
	if(Name)
	{
		SerialPrint(Name);
		ACPI_FREE(NameBuffer.Pointer);
	}

    // Get the device info
    Status = AcpiGetObjectInfo(Object, &DeviceInfo);
    if (ACPI_FAILURE(Status)) {
        return (Status);
    }

	if(DeviceInfo->HardwareId.String)
	{
		SerialPrint(" HIDString: ");
		SerialPrint(DeviceInfo->HardwareId.String);
	}
	if(DeviceInfo->UniqueId.String)
	{
		SerialPrint(" UIDString: ");
		SerialPrint(DeviceInfo->UniqueId.String);
	}

	const char* Type = AcpiUtGetTypeName(DeviceInfo->Type);

	
	SerialPrint(" Type: ");
	SerialPrint(Type);

	ReadAcpiBuffer(Object, "_HID");
	ReadAcpiBuffer(Object, "_HIDSTR");
	ReadAcpiBuffer(Object, "_CLS");
	ReadAcpiBuffer(Object, "_CID");
	ReadAcpiBuffer(Object, "_UID");
	ReadAcpiBuffer(Object, "_STR");
	ReadAcpiBuffer(Object, "_SxD");
	ReadAcpiBuffer(Object, "_SXDS");
	ReadAcpiBuffer(Object, "_SxW");
	ReadAcpiBuffer(Object, "_STA");
	ReadAcpiBuffer(Object, "_SXWS");
	ReadAcpiBuffer(Object, "_ADR");
	ReadAcpiBuffer(Object, "_DDN");
	ReadAcpiBuffer(Object, "_HRV");
	ReadAcpiBuffer(Object, "_MLS");
	ReadAcpiBuffer(Object, "_PLD");
	ReadAcpiBuffer(Object, "_SUB");
	ReadAcpiBuffer(Object, "_SUN");
	ReadAcpiBuffer(Object, "_STR");
	ReadAcpiBuffer(Object, "_CRS");
	ReadAcpiBuffer(Object, "_DMA");
	ReadAcpiBuffer(Object, "_DIS");
	ReadAcpiBuffer(Object, "_DSD");
	ReadAcpiBuffer(Object, "_HPX");
	ReadAcpiBuffer(Object, "_MAT");
	//ReadAcpiBuffer(Object, "_OSC");

	// https://github.com/qemu/qemu/blob/fd9a38fd437c4c31705071c240f4be11394ca1f8/hw/i386/acpi-build.c#L1270

	 /* DRAM controller */
    //dev = aml_device("DRAC");
    //aml_append(dev, aml_name_decl("_HID", aml_string("PNP0C01")));

	/*void xhci_sysbus_build_aml(Aml *scope, uint32_t mmio, unsigned int irq)
{
    Aml *dev = aml_device("XHCI");
    Aml *crs = aml_resource_template();

    aml_append(crs, aml_memory32_fixed(mmio, XHCI_LEN_REGS, AML_READ_WRITE));
    aml_append(crs, aml_interrupt(AML_CONSUMER, AML_LEVEL, AML_ACTIVE_HIGH,
                                  AML_EXCLUSIVE, &irq, 1));

    aml_append(dev, aml_name_decl("_HID", aml_eisaid("PNP0D10")));
    aml_append(dev, aml_name_decl("_CRS", crs));*/

	SerialPrint("\n");

    ACPI_FREE(DeviceInfo);

    return (AE_OK);
}

void WalkAcpiTree()
{
    AcpiWalkNamespace(ACPI_TYPE_ANY, ACPI_ROOT_OBJECT,
                      ACPI_UINT32_MAX, AcpiDeviceTreeCallback,
                      NULL, NULL, NULL);
}

struct DeviceToFind
{
	uint32_t HID;
	uint32_t UID;
};

static ACPI_STATUS AcpiSpecificDeviceCallback(ACPI_HANDLE Device, UINT32 Level, void *Context, void **ReturnValue)
{
	ACPI_DEVICE_INFO* deviceInfo;

	DeviceToFind* device = (DeviceToFind*)Context;

	ACPI_STATUS status = AcpiGetObjectInfo(Device, &deviceInfo);
	if (ACPI_FAILURE(status))
	{
		return status;
	}

	_ASSERTFV((device->HID & 0xFFFF) == 0x41D0, "_HID doesn't match expected PNP", device->HID & 0xFFFF, 0, 0);

	uint32_t deviceHID = (device->HID >> 16);
	char hidString[16];

	int shift = 0;
	hidString[0] = ((device->HID >> shift) & 0x1F) + 'A' - 1; shift+=5;
	hidString[1] = ((device->HID >> shift) & 0x1F) + 'A' - 1; shift+=5;
	hidString[2] = ((device->HID >> shift) & 0x1F) + 'A' - 1;

    witoabuf(hidString+3, deviceHID, 16, 4);

	char uidString[16];
    witoabuf(uidString, (uint32_t)device->UID, 10);

	if(deviceInfo->HardwareId.String)
	{
		bool matchUid = false;
		if(deviceInfo->UniqueId.String)
		{
			matchUid = _stricmp(deviceInfo->UniqueId.String, uidString) == 0;
		}
		else
		{
			matchUid = device->UID == 0;
		}

		bool match = _stricmp(deviceInfo->HardwareId.String, hidString) == 0 && matchUid;

		if(match)
		{
			*ReturnValue = Device;
			AcpiOsFree(deviceInfo);
			return AE_CTRL_TERMINATE;
		}
	}

	AcpiOsFree(deviceInfo);
	return AE_OK;
}

bool FindSpecificDevice(uint32_t hid, uint32_t uid, ACPI_HANDLE *outDevice)
{
	ACPI_STATUS status;
	ACPI_HANDLE parent = ACPI_ROOT_OBJECT;
	ACPI_BUFFER buffer;
	ACPI_HANDLE currentDevice = nullptr;
	ACPI_DEVICE_INFO *deviceInfo;

	DeviceToFind device = { hid, uid };

	status = AcpiGetDevices(NULL, AcpiSpecificDeviceCallback, &device, (void **)&currentDevice);

	if (ACPI_SUCCESS(status) && currentDevice != NULL)
	{
		*outDevice = currentDevice;
		return true;
	}

	return false;
}

ACPI_STATUS GetPciIdFromAcpiHandle(ACPI_HANDLE Handle, ACPI_PCI_ID *PciId)
{
    ACPI_STATUS Status = AE_OK;
    ACPI_DEVICE_INFO *DeviceInfo;
    ACPI_HANDLE ParentHandle = Handle;

    // Ensure valid input
    _ASSERTF(Handle != NULL, "Handle must not be NULL");
    _ASSERTF(PciId != NULL, "PciId must not be NULL");

    /*// Traverse up the ACPI tree to find the PCI device
    while (ParentHandle)
    {
        Status = AcpiGetObjectInfo(ParentHandle, &DeviceInfo);
        if (ACPI_FAILURE(Status))
        {
            SerialPrint("Error: Failed to get ACPI object info\n");
            return Status;
        }

        // Check if the device is a PCI/PCIe device
        if ((DeviceInfo->Valid & ACPI_VALID_ADR) && (DeviceInfo->Address != ACPI_ADR_SPACE_PCI))
        {
            PciId->Bus = ACPI_PCI_BUS(DeviceInfo->Address);
            PciId->Device = ACPI_PCI_DEVICE(DeviceInfo->Address);
            PciId->Function = ACPI_PCI_FUNCTION(DeviceInfo->Address);

            AcpiOsFree(DeviceInfo);
            return AE_OK;
        }

        AcpiOsFree(DeviceInfo);

        // Move to the parent ACPI object
        Status = AcpiGetParent(ParentHandle, &ParentHandle);
        if (ACPI_FAILURE(Status))
        {
            SerialPrint("Error: Failed to get parent ACPI object\n");
            return Status;
        }
    }*/

	 Status = AcpiGetObjectInfo(ParentHandle, &DeviceInfo);
	if (ACPI_FAILURE(Status))
	{
		SerialPrint("Error: Failed to get ACPI object info\n");
		return Status;
	}

	// Check if the device is a PCI/PCIe device
	if ((DeviceInfo->Valid & ACPI_VALID_ADR))
	{
		PciId->Bus = ACPI_PCI_BUS(DeviceInfo->Address);
		PciId->Device = ACPI_PCI_DEVICE(DeviceInfo->Address);
		PciId->Function = ACPI_PCI_FUNCTION(DeviceInfo->Address);

		AcpiOsFree(DeviceInfo);
		return AE_OK;
	}

	AcpiOsFree(DeviceInfo);

    return AE_NOT_FOUND;
}

unsigned int DevicePathNodeLength(const EFI_DEVICE_PATH_PROTOCOL *DevicePath)
{
	return ((unsigned int)DevicePath->Length[0]) | ((unsigned int)DevicePath->Length[1] << 8);
}

EFI_DEVICE_PATH_PROTOCOL* NextDevicePathNode(EFI_DEVICE_PATH_PROTOCOL *DevicePath)
{
	return (EFI_DEVICE_PATH_PROTOCOL*)((UINT8*)DevicePath + DevicePathNodeLength(DevicePath));
}