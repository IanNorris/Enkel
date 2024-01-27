#pragma once

//Acronyms used:
//FIS: Frame information structure
//DMA: Direct memory access
//PRDT: Physical region descriptor table

// AHCI and SATA related constants and structures
#define AHCI_PORT_CMD_ST 0x0001
#define AHCI_PORT_CMD_FRE 0x0010
#define AHCI_PORT_CMD_FR 0x4000
#define AHCI_PORT_CMD_CR 0x8000
#define AHCI_PORT_CMD_ISSUE 0x1000000

#define AHCI_CLASS_CODE 0x01
#define AHCI_SUBCLASS_CODE 0x06
#define AHCI_PROG_IF 0x01
#define PCI_BAR5_OFFSET 0x24

// Standard AHCI and SATA definitions
#define SATA_SIG_ATAPI 0xEB140101 // SATA signature for ATAPI devices
#define HBA_PORT_DET_PRESENT 0x3
#define HBA_PORT_IPM_ACTIVE 0x1

#define HBA_PxIE_DHR 0x80000000 // Device to host register FIS interrupt enable
#define HBA_PxIS_TFES 0x40000000 // Task file error status
#define ATA_CMD_IDENTIFY 0xEC
#define ATA_CMD_IDENTIFY_ATAPI 0xA1
#define ATA_CMD_PACKET 0xA0
#define ATA_CMD_READ_DMA_EX 0x25

#define PCI_BAR5_OFFSET 0x24

// https://wiki.osdev.org/ATAPI#Complete_Command_Set
#define ATAPI_CMD_READ_10 0x28
#define ATAPI_CMD_READ_12 0xA8

#define ATA_IDENTIFY_DEVICETYPE   		0
#define ATA_IDENTIFY_SERIAL       		20
#define ATA_IDENTIFY_BYTES_PER_SECTOR   120
#define ATA_IDENTIFY_FIRMWARE_REV 		46
#define ATA_IDENTIFY_MODEL        		54
#define ATA_IDENTIFY_CAPABILITIES 		98
#define ATA_IDENTIFY_MAX_LBA      		120
#define ATA_IDENTIFY_COMMANDSETS  		164
#define ATA_IDENTIFY_MAX_LBA_EXT  		200

#define STS_BSY (1 << 7) //Indicates the interface is busy
#define STS_DRQ (1 << 3) //Indicates a data transfer is requested
#define STS_ERR (1 << 0) //Indicates an error during the transfer.
#define STS_DRDY (1 << 6) //Indicates the drive is ready to accept commands

#define IS_ERR_FATAL (1 << 30)

#define PCI_COMMAND_INTERRUPT_DISABLE (1 << 10)
#define PCI_COMMAND_BUS_MASTER (1 << 2)
#define PCI_COMMAND_MEMORY_SPACE (1 << 1)
#define PCI_COMMAND_IO_SPACE (1 << 0)

#define HBA_PxCMD_ST (1 << 0) //Start
#define HBA_PxCMD_SUD (1 << 1) //Spin-up device
#define HBA_PxCMD_POD (1 << 2) //Power on device
#define HBA_PxCMD_CLO (1 << 3) //Command list override
#define HBA_PxCMD_FRE (1 << 4) //FIS receive enable
#define HBA_PxCMD_MPSS (1 << 13) //Mechanical switch state
#define HBA_PxCMD_FR (1 << 14) //FIS receive running
#define HBA_PxCMD_CR (1 << 15) //Command list running
#define HBA_PxCMD_CPS (1 << 16) //Cold presence state
#define HBA_PxCMD_PMA (1 << 17) //Port multiplier attached
#define HBA_PxCMD_HPCP (1 << 18) //Hot plug capable
#define HBA_PxCMD_MPSP (1 << 19) //Mechanical switch attached to port
#define HBA_PxCMD_CPD (1 << 20) //Cold presence detect
#define HBA_PxCMD_ESP (1 << 21) //External SATA port
#define HBA_PxCMD_FBSCP (1 << 22) //FIS-based switching capable port
#define HBA_PxCMD_APSTE (1 << 23) //Automatic partial to slumber transitions enabled
#define HBA_PxCMD_ATAPI (1 << 24) //Device is ATAPI
#define HBA_PxCMD_DLAE (1 << 25) //Device LED on ATAPI enable
#define HBA_PxCMD_ALPE (1 << 26) //Aggressive link power management enable
#define HBA_PxCMD_ASP (1 << 27) //Aggressive slumber/power management enable


enum class FISType : uint8_t
{
	RegisterHostToDevice		= 0x27,
	RegisterDeviceToHost		= 0x34,
	DMAActivate					= 0x39,
	SetupDMA					= 0x41,
	Data						= 0x46,
	BuiltInSelfTest				= 0x58,
	SetupProgrammedIO			= 0x5F,
	SetupDeviceBits				= 0xA1,
};

struct FISSetupDMA
{
    uint8_t FISType;          // FIS Type
    uint8_t PmPort:4;         // Port Multiplier Port
    uint8_t Reserved0:1;      // Reserved
    uint8_t Direction:1;      // Data direction: 1 for device to host, 0 for host to device
    uint8_t Interrupt:1;      // Interrupt bit
    uint8_t AutoActivate:1;   // Auto-activate bit

    uint8_t Reserved1[2];     // Reserved

    uint64_t DmaBufferId;     // DMA Buffer Identifier

    uint32_t Reserved2;       // Reserved

    uint32_t DmaBufferOffset; // DMA Buffer Offset
    uint32_t TransferCount;   // Number of bytes to transfer
    uint32_t Reserved3;       // Reserved
} PACKED_ALIGNMENT;

struct HBAPRDTEntry
{
    uint32_t DataBaseLow;       	// Data base address
    uint32_t DataBaseHigh;  		// Data base address upper 32 bits
    uint32_t Reserved0;             // Reserved
    uint32_t ByteCount : 22;        // Byte count, 4M max
    uint32_t Reserved1 : 9;         // Reserved
    uint32_t InterruptOnCompletion : 1; // Interrupt on completion
} PACKED_ALIGNMENT;

struct HBACommandListHeader
{
    uint8_t  CommandFISLength : 5;  // Command FIS length in DWORDS, 2 ~ 16
    uint8_t  Atapi : 1;             // ATAPI
    uint8_t  Write : 1;             // Write, 1: H2D, 0: D2H
    uint8_t  Prefetchable : 1;      // Prefetchable
    uint8_t  Reset : 1;             // Reset
    uint8_t  Bist : 1;              // BIST
    uint8_t  ClearBusyUponR_OK : 1; // Clear busy upon R_OK
    uint8_t  Reserved0 : 1;         // Reserved
    uint8_t  PortMultiplierPort : 4;// Port multiplier port
    uint16_t PRDTLength;            // Physical region descriptor table length in entries
    volatile uint32_t PRDByteCount; // Physical region descriptor byte count transferred
    uint32_t CommandTableBaseLow;   // Command table descriptor base address
    uint32_t CommandTableBaseHigh;  // Command table descriptor base address upper 32 bits

    uint32_t Reserved1[4];         // Reserved
} PACKED_ALIGNMENT;

struct HBACommandTable
{
    uint8_t CommandFIS[64];         // Command FIS
    uint8_t AtapiCommand[16];       // ATAPI command
    uint8_t Reserved[48];           // Reserved
    HBAPRDTEntry PRDTEntries[1];    // 1 PRDT entry (up to 65535)
} PACKED_ALIGNMENT;

struct HBAPort
{
    uint32_t CommandListBaseLow; // 0x00, command list base address, 1K-byte aligned
    uint32_t CommandListBaseHigh; // 0x04, upper 32 bits of command list base address
    uint32_t FISBaseLow; // 0x08, FIS base address, 256-byte aligned
    uint32_t FISBaseHigh; // 0x0C, upper 32 bits of FIS base address
    uint32_t InterruptStatus; // 0x10, interrupt status
    uint32_t InterruptEnable; // 0x14, interrupt enable
    uint32_t CommandAndStatus; // 0x18, command and status
    uint32_t Reserved0; // 0x1C, Reserved
    uint32_t TaskFileData; // 0x20, task file data
    uint32_t Signature; // 0x24, signature
    uint32_t SATAStatus; // 0x28, SATA status (SCR0:SStatus)
    uint32_t SATAControl; // 0x2C, SATA control (SCR2:SControl)
    uint32_t SATAError; // 0x30, SATA error (SCR1:SError)
    uint32_t SATAActive; // 0x34, SATA active (SCR3:SActive)
    uint32_t CommandIssue; // 0x38, command issue
    uint32_t SATANotification; // 0x3C, SATA notification (SCR4:SNotification)
    uint32_t FISSwitchControl; // 0x40, FIS-based switch control
    uint32_t Reserved1[11]; // 0x44 ~ 0x6F, Reserved
    uint32_t VendorSpecific[4]; // 0x70 ~ 0x7F, vendor specific
} PACKED_ALIGNMENT;

struct HBAMemory
{
    uint32_t HostCapability;
    uint32_t GlobalHostControl;
    uint32_t InterruptStatus;
    uint32_t PortsImplemented;
    uint32_t Version;
    uint32_t CommandCompletionCoalescingControl;
    uint32_t CommandCompletionCoalescingPorts;
    uint32_t EnclosureManagementLocation;
    uint32_t EnclosureManagementControl;
    uint32_t HostCapabilitiesExtended;
    uint32_t BIOSHandoffControlStatus;
    uint8_t  Reserved[0x74];
    uint8_t  VendorSpecific[0x60];

    HBAPort Ports[0]; // 0x100 onwards (not all ports will be available)
} PACKED_ALIGNMENT;

struct FISRegisterHostToDevice
{
    uint8_t  FISType;        // FIS_TYPE_REG_H2D
    uint8_t  PortMultiplier : 4; // Port multiplier
    uint8_t  Reserved0 : 3;      // Reserved
    uint8_t  CommandControl : 1; // 1: Command, 0: Control

    uint8_t  Command;       // Command register
    uint8_t  FeatureLow;    // Feature register, 7:0

    uint8_t  LBA0;          // LBA low register, 7:0
    uint8_t  LBA1;          // LBA mid register, 15:8
    uint8_t  LBA2;          // LBA high register, 23:16
    uint8_t  Device;        // Device register

    uint8_t  LBA3;          // LBA register, 31:24
    uint8_t  LBA4;          // LBA register, 39:32
    uint8_t  LBA5;          // LBA register, 47:40
    uint8_t  FeatureHigh;   // Feature register, 15:8

    uint8_t  CountLow;      // Count register, 7:0
    uint8_t  CountHigh;     // Count register, 15:8
    uint8_t  IsochronousCommandCompletion; // Isochronous command completion
    uint8_t  Control;       // Control register

    uint8_t  Reserved1[4];  // Reserved
} PACKED_ALIGNMENT;

struct FISRegisterDeviceToHost
{
    uint8_t  FISType;    // FIS_TYPE_REG_D2H
    uint8_t  PortMultiplier : 4; // Port multiplier
    uint8_t  Reserved0 : 2;      // Reserved
    uint8_t  Interrupt : 1;      // Interrupt bit
    uint8_t  Reserved1 : 1;      // Reserved

    uint8_t  Status;     // Status register
    uint8_t  Error;      // Error register

    uint8_t  LBA0;       // LBA low register, 7:0
    uint8_t  LBA1;       // LBA mid register, 15:8
    uint8_t  LBA2;       // LBA high register, 23:16
    uint8_t  Device;     // Device register

    uint8_t  LBA3;       // LBA register, 31:24
    uint8_t  LBA4;       // LBA register, 39:32
    uint8_t  LBA5;       // LBA register, 47:40
    uint8_t  Reserved2;  // Reserved

    uint8_t  CountLow;   // Count register, 7:0
    uint8_t  CountHigh;  // Count register, 15:8
    uint8_t  Reserved3[2]; // Reserved

    uint8_t  Reserved4[4]; // Reserved
} PACKED_ALIGNMENT;

struct FIS
{
    uint8_t  FISType;    // FIS_TYPE_DATA
    uint8_t  PortMultiplier : 4; // Port multiplier
    uint8_t  Reserved0 : 4;      // Reserved

    uint8_t  Reserved1[2]; // Reserved

    uint32_t Data[1]; // Variable-sized data payload
} PACKED_ALIGNMENT;

struct FISSetupProgrammedIO
{
    uint8_t FISType;          // FIS Type
    uint8_t PortMultiplier:4; // Port Multiplier Port
    uint8_t Reserved0:1;      // Reserved
    uint8_t Direction:1;      // Data direction: 1 for device to host, 0 for host to device
    uint8_t Interrupt:1;      // Interrupt bit
    uint8_t Reserved1:1;      // Reserved

    uint8_t Status;           // Status register
    uint8_t Error;            // Error register

    uint8_t LbaLow;           // LBA low register
    uint8_t LbaMid;           // LBA mid register
    uint8_t LbaHigh;          // LBA high register
    uint8_t Device;           // Device register

    uint8_t LbaLowExp;        // LBA low extended register
    uint8_t LbaMidExp;        // LBA mid extended register
    uint8_t LbaHighExp;       // LBA high extended register
    uint8_t Reserved2;        // Reserved

    uint8_t SectorCount;      // Sector count register
    uint8_t SectorCountExp;   // Sector count extended register
    uint8_t Reserved3;        // Reserved
    uint8_t E_Status;         // E_Status register

    uint16_t TransferCount;   // Transfer count
    uint16_t Reserved4;       // Reserved
} PACKED_ALIGNMENT;

struct FISSetupDevBits
{
    uint8_t FISType;         // FIS Type
    uint8_t PortMultiplier:4;// Port Multiplier Port
    uint8_t Reserved0:2;     // Reserved
    uint8_t Interrupt:1;     // Interrupt bit
    uint8_t Notification:1;  // Notification bit

    uint8_t Status;          // Status register
    uint8_t Error;           // Error register

    uint32_t SActive;        // Active Set
} PACKED_ALIGNMENT;

struct FISData
{
	FISSetupDMA SetupDMA;
	uint8_t Pad0[4];
 
	FISSetupProgrammedIO SetupProgrammedIO;
	uint8_t Pad1[12];
 
	FISRegisterDeviceToHost RegisterDeviceToHost;
	uint8_t Pad2[4];
 
 	FISSetupDevBits SetupDevBits;
 
	uint8_t Unused[64];
 
	uint8_t Reserved[60];
} PACKED_ALIGNMENT;

static_assert(sizeof(HBACommandListHeader) == 32, "HBACommandListHeader structure size must be 32 bytes.");
static_assert(sizeof(HBACommandTable) >= 128, "HBACommandTable structure size must be at least 128 bytes.");
static_assert(sizeof(HBAPort) == 128, "HBAPort structure size must be 128 bytes.");
static_assert(sizeof(HBAMemory) == 256, "Incorrect size for HBAMemory");
static_assert(sizeof(FISRegisterHostToDevice) == 20, "FISRegisterHostToDevice structure size must be 20 bytes.");
static_assert(sizeof(FISRegisterDeviceToHost) == 20, "FISRegisterDeviceToHost structure size must be 20 bytes.");

int FindCommandSlot(HBAPort *port);
int ReadFromAHCIPort(HBAPort *port, uint64_t startl, uint64_t starth, uint32_t count, uint8_t *buf);
