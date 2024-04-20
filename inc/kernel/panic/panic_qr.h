#pragma once

#include "common/types.h"

#define QR_MAGIC_BYTE (0x2D)
#define QR_VERSION (0x00)
#define QR_HEADER_PAD (0xCAFEF00D)

namespace Panic
{
	enum class EnkelStateBits
	{
		UserMode,
		KernelGSActive,
		Syscall,
		Interrupt, //An interrupt (other than the exception handler calling this)
	};

	struct Header
	{
		uint8_t MagicByte;
		uint8_t Version;
		uint8_t Page;
		uint8_t PageCount;
		uint32_t Pad;
	} PACKED_ALIGNMENT;

	struct CPURegs
	{
		uint64_t RAX; //0x00
		uint64_t RBX; //0x08
		uint64_t RCX; //0x10
		uint64_t RDX; //0x18
		uint64_t RSI; //0x20
		uint64_t RDI; //0x28

		uint64_t R8;  //0x30
		uint64_t R9;  //0x38
		uint64_t R10; //0x40
		uint64_t R11; //0x48
		uint64_t R12; //0x50
		uint64_t R13; //0x58
		uint64_t R14; //0x60
		uint64_t R15; //0x68

		uint64_t RIP; //0x70
		uint64_t RSP; //0x78
		uint64_t RBP; //0x80
		uint64_t RFLAGS; //0x88

		uint64_t CR0; //0x90
		uint64_t CR2; //0x98
		uint64_t CR3; //0xA0
		uint64_t CR4; //0xA8

		uint64_t GDTR; //0xB0
		uint64_t LDTR; //0xB8
		uint64_t IDTR; //0xC0
		uint64_t TR;   //0xC8
		
		uint64_t FSBase; //0xD0
		uint64_t GSBase; //0xD8
		uint64_t GSBaseOther; //0xE0
		
		uint16_t CS; //0xE8
		uint16_t DS; //0xEA
		uint16_t SS; //0xEC
		uint16_t ES; //0xEE

		uint16_t FS; //0xF0
		uint16_t GS; //0xF2

		uint16_t EnkelStateBits; //0xF4
		uint8_t  ISR[2]; //ISR stack //0xF6,0xF7
	} PACKED_ALIGNMENT;
}
