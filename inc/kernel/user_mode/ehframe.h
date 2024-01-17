#pragma once

#include "common/types.h"

enum DWARF_EH_PE_Encoding : uint8_t
{
	DW_EH_PE_omit 		= 0xff,
	DW_EH_PE_uleb128	= 0x01,
	DW_EH_PE_udata2		= 0x02,
	DW_EH_PE_udata4		= 0x03,
	DW_EH_PE_udata8		= 0x04,

	DW_EH_PE_sleb128	= 0x09,
	DW_EH_PE_sdata2		= 0x0a,
	DW_EH_PE_sdata4		= 0x0b,
	DW_EH_PE_sdata8		= 0x0c,
};

struct Elf64_EhFrameHdr_Header
{
	uint8_t Version;
	uint8_t EhFramePtrEnc;
	uint8_t FDECountEnc;
	uint8_t TableEnc;
	uint32_t EhFramePtr;
	uint32_t TableEntryCount; //TODO: This encoding is variable (in theory)
} PACKED_ALIGNMENT;

//TODO: This encoding is variable (in theory)
struct Elf64_EhFrameHdrEntry
{
	int32_t StartAddress;
	int32_t FDEAddress;
} PACKED_ALIGNMENT;

