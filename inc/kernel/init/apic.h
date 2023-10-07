#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef NULL
#undef NULL
#endif
#include "Uefi.h"
#ifdef NULL
#undef NULL
#endif
#define NULL nullptr

#include "Uefi.h"
#include "IndustryStandard/Acpi61.h"
#include "Protocol/AcpiSystemDescriptionTable.h"

enum class LocalApicOffsets : uint32_t
{
    LocalApicIdRegister = 0x020,
    LocalApicVersionRegister = 0x030,
    TaskPriorityRegister = 0x080,
    ArbitrationPriorityRegister = 0x090,
    ProcessorPriorityRegister = 0x0A0,
    EoiRegister = 0x0B0,
    RemoteReadRegister = 0x0C0,
    LogicalDestinationRegister = 0x0D0,
    DestinationFormatRegister = 0x0E0,
    SpuriousInterruptVectorRegister = 0x0F0,
    InServiceRegister = 0x100,
    TriggerModeRegister = 0x180,
    InterruptRequestRegister = 0x200,
    ErrorStatusRegister = 0x280,
    LvtCorrectedMachineCheckInterruptRegister = 0x2F0,
    InterruptCommandRegisterLow = 0x300,
    InterruptCommandRegisterHigh = 0x310,
    LvtTimerRegister = 0x320,
    LvtThermalSensorRegister = 0x330,
    LvtPerformanceMonitoringCountersRegister = 0x340,
    LvtLint0Register = 0x350,
    LvtLint1Register = 0x360,
    LvtErrorRegister = 0x370,
    InitialCountRegister = 0x380,
    CurrentCountRegister = 0x390,
    DivideConfigurationRegister = 0x3E0
};


void InitApic(EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER* Rsdt, EFI_ACPI_DESCRIPTION_HEADER* Xsdt);
