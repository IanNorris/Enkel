/******************************************************************************
 *
 * Module Name: osenkel - Enkel OSL
 *
 *****************************************************************************/

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

#include "kernel/init/apic.h"
#include "kernel/init/gdt.h"
#include "kernel/init/pic.h"
#include "kernel/init/msr.h"
#include "kernel/init/interrupts.h"
#include "kernel/console/console.h"
#include "kernel/devices/pci.h"
#include "memory/physical.h"
#include "kernel/scheduling/time.h"
#include "utilities/termination.h"

#include "rpmalloc.h"

extern "C"
{
#include "acpi.h"
#include "accommon.h"
#include "amlresrc.h"

#define LOG_DBG(msg)

static bool EnableDebugLogging;

static ACPI_PHYSICAL_ADDRESS RsdpPhyAdd;

#include "kernel/init/bootload.h"

#define FALSE (0)
#define TRUE (1)

extern KernelBootData GBootData;



static void sys_out8(uint8_t data, uint16_t port)
{
        __asm__ volatile("outb %b0, %w1" :: "a"(data), "Nd"(port));
}

static void sys_out16(uint16_t data, uint16_t port)
{
        __asm__ volatile("outw %w0, %w1" :: "a"(data), "Nd"(port));
}

static void sys_out32(uint32_t data, uint16_t port)
{
        __asm__ volatile("outl %0, %w1" :: "a"(data), "Nd"(port));
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsReadable
 *
 * PARAMETERS:  Pointer             - Area to be verified
 *              Length              - Size of area
 *
 * RETURN:      TRUE if readable for entire Length
 *
 * DESCRIPTION: Verify that a pointer is valid for reading
 *
 *****************************************************************************/

BOOLEAN
AcpiOsReadable (
    void                    *Pointer,
    ACPI_SIZE               Length)
{
    return (TRUE);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiEnableDbgPrint
 *
 * PARAMETERS:  en, 	            - Enable/Disable debug print
 *
 * RETURN:      None
 *
 * DESCRIPTION: Formatted output
 *
 *****************************************************************************/

void
AcpiEnableDbgPrint (
    bool                    Enable)
{
    if (Enable)
    {
        EnableDebugLogging = true;
    }
    else
    {
        EnableDebugLogging = false;
    }
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsPrintf
 *
 * PARAMETERS:  Fmt, ...            - Standard printf format
 *
 * RETURN:      None
 *
 * DESCRIPTION: Formatted output
 *
 *****************************************************************************/

void ACPI_INTERNAL_VAR_XFACE
AcpiOsPrintf (
    const char              *Fmt,
    ...)
{
    va_list                 args;

    va_start (args, Fmt);

    if (EnableDebugLogging)
    {
		SerialPrint(Fmt);
		//TODO
        //printk (Fmt, args);
    }

    va_end (args);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsGetLine
 *
 * PARAMETERS:  Buffer              - Where to return the command line
 *              BufferLength        - Maximum Length of Buffer
 *              BytesRead           - Where the actual byte count is returned
 *
 * RETURN:      Status and actual bytes read
 *
 * DESCRIPTION: Formatted input with argument list pointer
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsGetLine (
    char                    *Buffer,
    UINT32                  BufferLength,
    UINT32                  *BytesRead)
{
    return (-1);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsAllocate
 *
 * PARAMETERS:  Size                - Amount to allocate, in bytes
 *
 * RETURN:      Pointer to the new allocation. Null on error.
 *
 * DESCRIPTION: Allocate memory. Algorithm is dependent on the OS.
 *
 *****************************************************************************/

void *
AcpiOsAllocate (
    ACPI_SIZE               Size)
{
	return rpmalloc(Size);
}


#ifdef USE_NATIVE_ALLOCATE_ZEROED
/******************************************************************************
 *
 * FUNCTION:    AcpiOsAllocateZeroed
 *
 * PARAMETERS:  Size                - Amount to allocate, in bytes
 *
 * RETURN:      Pointer to the new allocation. Null on error.
 *
 * DESCRIPTION: Allocate and zero memory. Algorithm is dependent on the OS.
 *
 *****************************************************************************/

void *
AcpiOsAllocateZeroed (
    ACPI_SIZE               Size)
{
    void *mem;

    mem = AcpiOsAllocate (Size);

    if (mem)
    {
        memset (mem, 0, Size);
    }

    return (mem);
}
#endif


/******************************************************************************
 *
 * FUNCTION:    AcpiOsFree
 *
 * PARAMETERS:  Mem                 - Pointer to previously allocated memory
 *
 * RETURN:      None.
 *
 * DESCRIPTION: Free memory allocated via AcpiOsAllocate
 *
 *****************************************************************************/

void
AcpiOsFree (
    void                    *Mem)
{
	rpfree(Mem);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsReadMemory
 *
 * PARAMETERS:  Address             - Physical Memory Address to read
 *              Value               - Where Value is placed
 *              Width               - Number of bits (8,16,32, or 64)
 *
 * RETURN:      Value read from physical memory Address. Always returned
 *              as a 64-bit integer, regardless of the read Width.
 *
 * DESCRIPTION: Read data from a physical memory Address
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsReadMemory (
    ACPI_PHYSICAL_ADDRESS   Address,
    UINT64                  *Value,
    UINT32                  Width)
{
	//TODO

    switch (Width)
    {
    case 8:

        //*((UINT8 *) Value) = sys_read8 (Address);
		*((UINT8 *) Value) = *(UINT8 *)(Address);
        break;

    case 16:

        *((UINT16 *) Value) = *(UINT16 *) (Address);
        break;

    case 32:

        *((UINT32 *) Value) = *(UINT32 *) (Address);
        break;
#if defined(__x86_64__)
    case 64:

        *((UINT64 *) Value) = *(UINT64 *) (Address);
        break;
#endif
    default:

        return (AE_BAD_PARAMETER);
    }

    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsWriteMemory
 *
 * PARAMETERS:  Address             - Physical Memory Address to write
 *              Value               - Value to write
 *              Width               - Number of bits (8,16,32, or 64)
 *
 * RETURN:      None
 *
 * DESCRIPTION: Write data to a physical memory Address
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsWriteMemory (
    ACPI_PHYSICAL_ADDRESS   Address,
    UINT64                  Value,
    UINT32                  Width)
{
    switch (Width)
    {
    case 8:

		*(UINT8*)Address = (UINT8)(Value & 0xFF);
        //sys_write8 ((UINT8) Value, Address);
        break;

    case 16:

        //sys_write16 ((UINT16) Value, Address);
		*(UINT16*)Address = (UINT16)(Value & 0xFFFF);
        break;

    case 32:

        //sys_write32 ((UINT32) Value, Address);
		*(UINT32*)Address = (UINT32)(Value & 0xFFFFFFFF);
        break;
#if defined(__x86_64__)
    case 64:

        //sys_write64 ((UINT64) Value, Address);
		*(UINT64*)Address = (UINT64)Value;
        break;
#endif
    default:

        return (AE_BAD_PARAMETER);
    }

    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsReadPort
 *
 * PARAMETERS:  Address             - Address of I/O port/register to read
 *              Value               - Where Value is placed
 *              Width               - Number of bits
 *
 * RETURN:      Value read from port
 *
 * DESCRIPTION: Read data from an I/O port or register
 *
 *****************************************************************************/

ACPI_STATUS AcpiOsReadPort(
    ACPI_IO_ADDRESS Address,
    UINT32 *Value,
    UINT32 Width)
{
    if (Value == nullptr) {
        return 1;  // Error code
    }

    switch (Width) {
    case 8:
        *Value = static_cast<UINT32>(InPort(static_cast<uint16_t>(Address)));
        break;
    case 16:
        *Value = static_cast<UINT32>(InPort(static_cast<uint16_t>(Address))) |
                 (static_cast<UINT32>(InPort(static_cast<uint16_t>(Address + 1))) << 8);
        break;
    case 32:
        *Value = static_cast<UINT32>(InPort(static_cast<uint16_t>(Address))) |
                 (static_cast<UINT32>(InPort(static_cast<uint16_t>(Address + 1))) << 8) |
                 (static_cast<UINT32>(InPort(static_cast<uint16_t>(Address + 2))) << 16) |
                 (static_cast<UINT32>(InPort(static_cast<uint16_t>(Address + 3))) << 24);
        break;
    default:
        return 2;  // Error code for unsupported width
    }

    return 0;  // Success code
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsWritePort
 *
 * PARAMETERS:  Address             - Address of I/O port/register to write
 *              Value               - Value to write
 *              Width               - Number of bits
 *
 * RETURN:      None
 *
 * DESCRIPTION: Write data to an I/O port or register
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsWritePort (
    ACPI_IO_ADDRESS         Address,
    UINT32                  Value,
    UINT32                  Width)
{
    switch (Width)
    {
    case 8:

        sys_out8 ((UINT8) Value, Address);
        break;

    case 16:

        sys_out16 ((UINT16) Value, Address);
        break;

    case 32:

        sys_out32 ((UINT32) Value, Address);
        break;

    case 64:

        sys_out32 ((UINT32) Value, Address);
        sys_out32 ((UINT32) (Value + 4), (Address + 4));
        break;

    default:

        return (AE_BAD_PARAMETER);
    }

    return (AE_OK);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsWritePciConfiguration
 *
 * PARAMETERS:  PciId               - Seg/Bus/Dev
 *              Register            - Device Register
 *              Value               - Value to be written
 *              Width               - Number of bits
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Write data to PCI configuration space
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsWritePciConfiguration (
    ACPI_PCI_ID             *PciId,
    UINT32                  Register,
    UINT64                  Value,
    UINT32                  Width)
{
	volatile UINT64* TargetAddress = PciConfigGetMMIOAddress(PciId->Bus, PciId->Device, PciId->Function, Register);

    switch (Width)
    {
    case 8:
		*((volatile UINT8*)TargetAddress) = (*((UINT8*)TargetAddress) & 0xffffffffffffff00) | (UINT8) Value;
        break;

    case 16:
		*((volatile UINT16*)TargetAddress) = (*((UINT16*)TargetAddress) & 0xffffffffffff0000) | (UINT16) Value;
        break;

    case 32:
		*((volatile UINT32*)TargetAddress) = (*((UINT32*)TargetAddress) & 0xffffffff00000000) | (UINT32) Value;
        break;

    case 64:
		*TargetAddress = Value;
        break;

    default:

        return (AE_BAD_PARAMETER);
    }

    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsReadPciConfiguration
 *
 * PARAMETERS:  PciId               - Seg/Bus/Dev
 *              Register            - Device Register
 *              Value               - Buffer Where Value is placed
 *              Width               - Number of bits
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Read data from PCI configuration space
 *
 *****************************************************************************/

ACPI_STATUS AcpiOsReadPciConfiguration(
    ACPI_PCI_ID *PciId,
    UINT32 Register,
    UINT64 *Value,
    UINT32 Width)
{
    // Validate parameters
    if (Value == NULL || PciId == NULL) {
        return AE_BAD_PARAMETER;
    }

    // Calculate the address in the PCI configuration space
    volatile UINT64* TargetAddress = PciConfigGetMMIOAddress(PciId->Bus, PciId->Device, PciId->Function, Register);
    if (TargetAddress == NULL) {
        return AE_ERROR;
    }

    volatile UINT64* VirtualAddress = (volatile UINT64*)AcpiOsMapMemory((ACPI_PHYSICAL_ADDRESS)TargetAddress, PAGE_SIZE);

    // Perform the read operation based on the specified width
    switch (Width) {
    case 8:
        *Value = *VirtualAddress;
        break;
    case 16:
        *Value = *(volatile UINT16*)VirtualAddress;
        break;
    case 32:
        *Value = *(volatile UINT32*)VirtualAddress;
        break;
    case 64:
        *Value = *VirtualAddress;
        break;
    }

    AcpiOsUnmapMemory((void*)VirtualAddress, PAGE_SIZE);

    return AE_OK;
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsRedirectOutput
 *
 * PARAMETERS:  Destination         - An open file handle/pointer
 *
 * RETURN:      None
 *
 * DESCRIPTION: Causes redirect of AcpiOsPrintf and AcpiOsVprintf
 *
 *****************************************************************************/

void
AcpiOsRedirectOutput (
    void                    *Destination)
{

}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsPredefinedOverride
 *
 * PARAMETERS:  InitVal             - Initial Value of the predefined object
 *              NewVal              - The new Value for the object
 *
 * RETURN:      Status, pointer to Value. Null pointer returned if not
 *              overriding.
 *
 * DESCRIPTION: Allow the OS to override predefined names
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsPredefinedOverride (
    const ACPI_PREDEFINED_NAMES *InitVal,
    ACPI_STRING                 *NewVal)
{

    if (!InitVal || !NewVal)
    {
        return (AE_BAD_PARAMETER);
    }

    *NewVal = NULL;

    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsTableOverride
 *
 * PARAMETERS:  ExistingTable       - Header of current table (probably firmware)
 *              NewTable            - Where an entire new table is returned.
 *
 * RETURN:      Status, pointer to new table. Null pointer returned if no
 *              table is available to override
 *
 * DESCRIPTION: Return a different version of a table if one is available
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsTableOverride (
    ACPI_TABLE_HEADER       *ExistingTable,
    ACPI_TABLE_HEADER       **NewTable)
{

    if (!ExistingTable || !NewTable)
    {
        return (AE_BAD_PARAMETER);
    }

    *NewTable = NULL;

    return (AE_NO_ACPI_TABLES);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsGetRootPointer
 *
 * PARAMETERS:  None
 *
 * RETURN:      RSDP physical Address
 *
 * DESCRIPTION: Gets the root pointer (RSDP)
 *
 *****************************************************************************/

ACPI_PHYSICAL_ADDRESS
AcpiOsGetRootPointer (
    void)
{
	 LOG_DBG ("");

	if(RsdpPhyAdd)
	{
		return RsdpPhyAdd;
	}

	RsdpPhyAdd = (ACPI_PHYSICAL_ADDRESS)GetPhysicalAddress((uint64_t)GBootData.Rsdt);

	return RsdpPhyAdd;
}

#ifndef ACPI_USE_NATIVE_MEMORY_MAPPING
/******************************************************************************
 *
 * FUNCTION:    AcpiOsMapMemory
 *
 * PARAMETERS:  Where               - Physical Address of memory to be mapped
 *              Length              - How much memory to map
 *
 * RETURN:      Pointer to mapped memory. Null on error.
 *
 * DESCRIPTION: Map physical memory into caller's Address space
 *
 *****************************************************************************/

void *
AcpiOsMapMemory (
    ACPI_PHYSICAL_ADDRESS   Where,
    ACPI_SIZE               Length)
{
    LOG_DBG ("");

	uint64_t offset = (Where - (Where & PAGE_MASK));
	return (uint8_t*)PhysicalAlloc(Where & PAGE_MASK, ((Length + offset) + PAGE_SIZE) & PAGE_MASK, PrivilegeLevel::Kernel, PageFlags_Cache_Disable) + offset;
}
#endif


/******************************************************************************
 *
 * FUNCTION:    AcpiOsUnmapMemory
 *
 * PARAMETERS:  Where               - Logical Address of memory to be unmapped
 *              Length              - How much memory to unmap
 *
 * RETURN:      None.
 *
 * DESCRIPTION: Delete a previously created mapping. Where and Length must
 *              correspond to a previous mapping exactly.
 *
 *****************************************************************************/

void
AcpiOsUnmapMemory (
    void                    *Where,
    ACPI_SIZE               Length)
{
    LOG_DBG ("");

	uint64_t whereAddr = (uint64_t)Where;
	
	uint64_t offset = (whereAddr - (whereAddr & PAGE_MASK));
	PhysicalFree((void*)(whereAddr & PAGE_MASK), ((Length + offset) + PAGE_SIZE) & PAGE_MASK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsPhysicalTableOverride
 *
 * PARAMETERS:  ExistingTable       - Header of current table (probably firmware)
 *              NewAddress          - Where new table Address is returned
 *                                    (Physical Address)
 *              NewTableLength      - Where new table Length is returned
 *
 * RETURN:      Status, Address/Length of new table. Null pointer returned
 *              if no table is available to override.
 *
 * DESCRIPTION: Returns AE_SUPPORT, function not used in user space.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsPhysicalTableOverride (
    ACPI_TABLE_HEADER       *ExistingTable,
    ACPI_PHYSICAL_ADDRESS   *NewAddress,
    UINT32                  *NewTableLength)
{

    LOG_DBG ("");
    return (AE_SUPPORT);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsInitialize
 *
 * PARAMETERS:  None
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Init this OSL
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsInitialize (
    void)
{
    LOG_DBG ("");
    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsStall
 *
 * PARAMETERS:  Microseconds        - Time to stall
 *
 * RETURN:      None. Blocks until stall is completed.
 *
 * DESCRIPTION: Sleep at microsecond granularity
 *
 *****************************************************************************/

void
AcpiOsStall (
    UINT32                  Microseconds)
{
    //k_busy_wait (Microseconds);
	NOT_IMPLEMENTED(); //TODO: Required on real hardware
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsSleep
 *
 * PARAMETERS:  Milliseconds        - Time to sleep
 *
 * RETURN:      None. Blocks until sleep is completed.
 *
 * DESCRIPTION: Sleep at millisecond granularity
 *
 *****************************************************************************/

void
AcpiOsSleep (
    UINT64                  Milliseconds)
{
    //k_msleep ((UINT32) Milliseconds);
	NOT_IMPLEMENTED();
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsEnterSleep
 *
 * PARAMETERS:  SleepState          - Which sleep state to enter
 *              RegaValue           - Register A Value
 *              RegbValue           - Register B Value
 *
 * RETURN:      Status
 *
 * DESCRIPTION: A hook before writing sleep registers to enter the sleep
 *              state. Return AE_CTRL_SKIP to skip further sleep register
 *              writes.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsEnterSleep (
    UINT8                   SleepState,
    UINT32                  RegaValue,
    UINT32                  RegbValue)
{
    //NOT_IMPLEMENTED();
    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsGetTimer
 *
 * PARAMETERS:  None
 *
 * RETURN:      Current ticks in 100-nanosecond units
 *
 * DESCRIPTION: Get the Value of a system timer
 *
 ******************************************************************************/

UINT64
AcpiOsGetTimer (
    void)
{
    //return (k_cycle_get_64 ());
	return _rdtsc();
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsInstallInterruptHandler
 *
 * PARAMETERS:  InterruptNumber     - Level handler should respond to.
 *              ServiceRoutine      - Address of the ACPI interrupt handler
 *              Context             - User context
 *
 * RETURN:      Handle to the newly installed handler.
 *
 * DESCRIPTION: Install an interrupt handler. Used to install the ACPI
 *              OS-independent handler.
 *
 *****************************************************************************/

UINT32
AcpiOsInstallInterruptHandler (
    UINT32                  InterruptNumber,
    ACPI_OSD_HANDLER        ServiceRoutine,
    void                    *Context)
{
    LOG_DBG ("");

	SetInterruptHandler(InterruptNumber, ServiceRoutine, Context);
    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsRemoveInterruptHandler
 *
 * PARAMETERS:  Handle              - Returned when handler was installed
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Uninstalls an interrupt handler.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsRemoveInterruptHandler (
    UINT32                  InterruptNumber,
    ACPI_OSD_HANDLER        ServiceRoutine)
{

    LOG_DBG ("");
    ClearInterruptHandler(InterruptNumber);
    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsSignal
 *
 * PARAMETERS:  Function            - ACPICA signal function code
 *              Info                - Pointer to function-dependent structure
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Miscellaneous functions. Example implementation only.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsSignal (
    UINT32                  Function,
    void                    *Info)
{
    switch (Function)
    {
    case ACPI_SIGNAL_FATAL:
        LOG_DBG ("ACPI_SIGNAL_FATAL error");
        break;

    case ACPI_SIGNAL_BREAKPOINT:
        LOG_DBG ("ACPI_SIGNAL_BREAKPOINT");
        break;

    default:
        break;
    }

    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    Spinlock/Semaphore interfaces
 *
 * DESCRIPTION: Map these interfaces to semaphore interfaces
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsCreateLock (
    ACPI_SPINLOCK           *OutHandle)
{
    LOG_DBG ("");

    return (AE_OK);
}

void
AcpiOsDeleteLock (
    ACPI_SPINLOCK           Handle)
{
    LOG_DBG ("");
}

ACPI_CPU_FLAGS
AcpiOsAcquireLock (
    ACPI_SPINLOCK           Handle)
{
    LOG_DBG ("");
    return (0);
}

void
AcpiOsReleaseLock (
    ACPI_SPINLOCK           Handle,
    ACPI_CPU_FLAGS          Flags)
{
    LOG_DBG ("");
}

ACPI_STATUS
AcpiOsCreateSemaphore (
    UINT32              MaxUnits,
    UINT32              InitialUnits,
    ACPI_HANDLE         *OutHandle)
{
    *OutHandle = (ACPI_HANDLE) 1;
    return (AE_OK);
}

ACPI_STATUS
AcpiOsDeleteSemaphore (
    ACPI_HANDLE         Handle)
{
    return (AE_OK);
}

ACPI_STATUS
AcpiOsWaitSemaphore (
    ACPI_HANDLE         Handle,
    UINT32              Units,
    UINT16              Timeout)
{
    return (AE_OK);
}

ACPI_STATUS
AcpiOsSignalSemaphore (
    ACPI_HANDLE         Handle,
    UINT32              Units)
{
    return (AE_OK);
}

ACPI_THREAD_ID
AcpiOsGetThreadId (
    void)
{
    LOG_DBG ("");
    return (1);
}

ACPI_STATUS
AcpiOsExecute (
    ACPI_EXECUTE_TYPE       Type,
    ACPI_OSD_EXEC_CALLBACK  Function,
    void                    *Context)
{
    Function (Context);
    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsVprintf
 *
 * PARAMETERS:  Fmt                 - Standard printf format
 *              Args                - Argument list
 *
 * RETURN:      None
 *
 * DESCRIPTION: Formatted output with argument list pointer
 *
 *****************************************************************************/

void
AcpiOsVprintf (
    const char              *Fmt,
    va_list                 Args)
{
	SerialPrint(Fmt);
	SerialPrint("\n");
	//NOT_IMPLEMENTED();

   /* INT32                   Count = 0;
    UINT8                   Flags;


    Flags = AcpiGbl_DbOutputFlags;
    if (Flags & ACPI_DB_REDIRECTABLE_OUTPUT)
    {
        // Output is directable to either a file (if open) or the console

        if (AcpiGbl_DebugFile)
        {
            // Output file is open, send the output there

            Count = vfprintf (AcpiGbl_DebugFile, Fmt, Args);
        }
        else
        {
            // No redirection, send output to console (once only!)

            Flags |= ACPI_DB_CONSOLE_OUTPUT;
        }
    }

    if (Flags & ACPI_DB_CONSOLE_OUTPUT)
    {
        Count = vfprintf (AcpiGbl_OutputFile, Fmt, Args);
    }*/

    return;
}

ACPI_STATUS
AcpiOsPurgeCache (
    ACPI_CACHE_T            *Cache)
{
	//NOT_IMPLEMENTED();

    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsWaitCommandReady
 *
 * PARAMETERS:  None
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Negotiate with the debugger foreground thread (the user
 *              thread) to wait the readiness of a command.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsWaitCommandReady (
    void)
{
	NOT_IMPLEMENTED();
    return (AE_OK);

 /*   ACPI_STATUS             Status = AE_OK;


    if (AcpiGbl_DebuggerConfiguration == DEBUGGER_MULTI_THREADED)
    {
        Status = AE_TIME;

        while (Status == AE_TIME)
        {
            if (AcpiGbl_DbTerminateLoop)
            {
                Status = AE_CTRL_TERMINATE;
            }
            else
            {
                Status = AcpiOsAcquireMutex (AcpiGbl_DbCommandReady, 1000);
            }
        }
    }
    else
    {
        // Force output to console until a command is entered 

        AcpiDbSetOutputDestination (ACPI_DB_CONSOLE_OUTPUT);

        // Different prompt if method is executing 

        if (!AcpiGbl_MethodExecuting)
        {
            AcpiOsPrintf ("%1c ", ACPI_DEBUGGER_COMMAND_PROMPT);
        }
        else
        {
            AcpiOsPrintf ("%1c ", ACPI_DEBUGGER_EXECUTE_PROMPT);
        }

        // Get the user input line

        Status = AcpiOsGetLine (AcpiGbl_DbLineBuf,
            ACPI_DB_LINE_BUFFER_SIZE, NULL);
    }

    if (ACPI_FAILURE (Status) && Status != AE_CTRL_TERMINATE)
    {
        ACPI_EXCEPTION ((AE_INFO, Status,
            "While parsing/handling command line"));
    }
    return (Status);*/
}

//Stubs

void
MpSaveGpioInfo (
    ACPI_PARSE_OBJECT       *Op,
    AML_RESOURCE            *Resource,
    UINT32                  PinCount,
    UINT16                  *PinList,
    char                    *DeviceName)
{
	NOT_IMPLEMENTED();
}

void
MpSaveSerialInfo (
    ACPI_PARSE_OBJECT       *Op,
    AML_RESOURCE            *Resource,
    char                    *DeviceName)
{
	NOT_IMPLEMENTED();
}

/*******************************************************************************
 *
 * FUNCTION:    AcpiAhMatchHardwareId
 *
 * PARAMETERS:  HardwareId          - String representation of an _HID or _CID
 *
 * RETURN:      ID info struct. NULL if HardwareId is not found
 *
 * DESCRIPTION: Lookup an _HID/_CID in the device ID table
 *
 ******************************************************************************/

const AH_DEVICE_ID *
AcpiAhMatchHardwareId (
    char                    *HardwareId)
{
	NOT_IMPLEMENTED();

    /*const AH_DEVICE_ID      *Info;


    for (Info = AslDeviceIds; Info->Name; Info++)
    {
        if (!strcmp (HardwareId, Info->Name))
        {
            return (Info);
        }
    }*/

    return (NULL);
}

ACPI_STATUS
AcpiOsCreateCache (
    char                    *CacheName,
    UINT16                  ObjectSize,
    UINT16                  MaxDepth,
    ACPI_CACHE_T            **ReturnCache)
{
    ACPI_MEMORY_LIST        *NewCache;


    NewCache = (ACPI_MEMORY_LIST*)rpmalloc (sizeof (ACPI_MEMORY_LIST));
    if (!NewCache)
    {
        return (AE_NO_MEMORY);
    }

    memset(NewCache, (uint8_t)0, sizeof (ACPI_MEMORY_LIST));
    NewCache->ListName = CacheName;
    NewCache->ObjectSize = ObjectSize;
    NewCache->MaxDepth = MaxDepth;

    *ReturnCache = (ACPI_CACHE_T*)NewCache;
    return (AE_OK);
}

ACPI_STATUS
AcpiOsDeleteCache (
    ACPI_CACHE_T            *Cache)
{
    rpfree (Cache);
    return (AE_OK);
}

void *
AcpiOsAcquireObject (
    ACPI_CACHE_T            *Cache)
{
    void                    *NewObject;

    NewObject = rpmalloc (((ACPI_MEMORY_LIST *) Cache)->ObjectSize);
    memset (NewObject, 0, ((ACPI_MEMORY_LIST *) Cache)->ObjectSize);

    return (NewObject);
}

ACPI_STATUS
AcpiOsReleaseObject (
    ACPI_CACHE_T            *Cache,
    void                    *Object)
{
    rpfree (Object);
    return (AE_OK);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsWaitEventsComplete
 *
 * PARAMETERS:  None
 *
 * RETURN:      None
 *
 * DESCRIPTION: Wait for all asynchronous events to complete. This
 *              implementation does nothing.
 *
 *****************************************************************************/

void
AcpiOsWaitEventsComplete (
    void)
{
	NOT_IMPLEMENTED();
    return;
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsTerminate
 *
 * PARAMETERS:  None
 *
 * RETURN:      Status
 *
 * DESCRIPTION: 
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsTerminate (
    void)
{
    return (AE_OK);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsNotifyCommandComplete
 *
 * PARAMETERS:  void
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Negotiate with the debugger foreground thread (the user
 *              thread) to notify the completion of a command.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsNotifyCommandComplete (
    void)
{
	NOT_IMPLEMENTED();
    /*if (AcpiGbl_DebuggerConfiguration == DEBUGGER_MULTI_THREADED)
    {
        AcpiOsReleaseMutex (AcpiGbl_DbCommandComplete);
    }*/
    return (AE_OK);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsInitializeDebugger
 *
 * PARAMETERS:  None
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Initialize OSPM specific part of the debugger
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsInitializeDebugger (
    void)
{
    //ACPI_STATUS             Status;

	NOT_IMPLEMENTED();

    /*// Create command signals

    Status = AcpiOsCreateMutex (&AcpiGbl_DbCommandReady);
    if (ACPI_FAILURE (Status))
    {
        return (Status);
    }
    Status = AcpiOsCreateMutex (&AcpiGbl_DbCommandComplete);
    if (ACPI_FAILURE (Status))
    {
        goto ErrorReady;
    }

    // Initialize the states of the command signals

    Status = AcpiOsAcquireMutex (AcpiGbl_DbCommandComplete,
        ACPI_WAIT_FOREVER);
    if (ACPI_FAILURE (Status))
    {
        goto ErrorComplete;
    }
    Status = AcpiOsAcquireMutex (AcpiGbl_DbCommandReady,
        ACPI_WAIT_FOREVER);
    if (ACPI_FAILURE (Status))
    {
        goto ErrorComplete;
    }

    AcpiGbl_DbCommandSignalsInitialized = TRUE;
    return (Status);

ErrorComplete:
    AcpiOsDeleteMutex (AcpiGbl_DbCommandComplete);
ErrorReady:
    AcpiOsDeleteMutex (AcpiGbl_DbCommandReady);
    return (Status);*/
}


/******************************************************************************
 *
 * FUNCTION:    AcpiOsTerminateDebugger
 *
 * PARAMETERS:  None
 *
 * RETURN:      None
 *
 * DESCRIPTION: Terminate signals used by the multi-threading debugger
 *
 *****************************************************************************/

void
AcpiOsTerminateDebugger (
    void)
{
	NOT_IMPLEMENTED();

    /*if (AcpiGbl_DbCommandSignalsInitialized)
    {
        AcpiOsDeleteMutex (AcpiGbl_DbCommandReady);
        AcpiOsDeleteMutex (AcpiGbl_DbCommandComplete);
    }*/
}

/*******************************************************************************
 *
 * FUNCTION:    AcpiAhMatchUuid
 *
 * PARAMETERS:  Data                - Data buffer containing a UUID
 *
 * RETURN:      ASCII description string for the UUID if it is found.
 *
 * DESCRIPTION: Returns a description string for "known" UUIDs, which are
 *              are UUIDs that are related to ACPI in some way.
 *
 ******************************************************************************/

const char *
AcpiAhMatchUuid (
    UINT8                   *Data)
{
    //const AH_UUID           *Info;
    //UINT8                   UuidBuffer[UUID_BUFFER_LENGTH];

	NOT_IMPLEMENTED();
/*
    // Walk the table of known ACPI-related UUIDs

    for (Info = Gbl_AcpiUuids; Info->Description; Info++)
    {
        // Null string means description is a UUID class 

        if (!Info->String)
        {
            continue;
        }

        AcpiUtConvertStringToUuid (Info->String, UuidBuffer);

        if (!memcmp (Data, UuidBuffer, UUID_BUFFER_LENGTH))
        {
            return (Info->Description);
        }
    }*/

    return (NULL);
}

}