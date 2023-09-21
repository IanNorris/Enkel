#pragma once

#include "common/types.h"

enum InterruptType : uint8_t
{
    InterruptGate = 0x8E,
    TrapGate = 0x8F
};


struct GDTEntry
{
    uint16_t LimitLow;          // The lower 16 bits of the limit.
    uint16_t BaseLow;           // The lower 16 bits of the base.
    uint8_t  BaseMiddle;        // The next 8 bits of the base.
    uint8_t  Accessed : 1;    // The accessed flag
    uint8_t  ReadWrite : 1;    // Read/Write flag (for code selectors)
    uint8_t  DirectionConforming : 1;    // Direction conforming flag (for code selectors - 0 - segment grows up, 1 - segment grows down). Controls if lower DPLs can access.
    uint8_t  Executable : 1;    // Executable flag
    uint8_t  DescriptorType : 1;    // Descriptor type (0 for system, 1 for code/data)
    uint8_t  Privilege : 2;    // Descriptor Privilege Level - ring 0 - 3.
    uint8_t  Present : 1;    // Present
    uint8_t  LimitHigh : 4;    // The upper 4 bits of the limit.
    uint8_t  Reserved : 1;    // Reserved for system use.
    uint8_t  LongModeCode : 1;    // Long mode for 64bit code segment.
    uint8_t  OpSize : 1;    // Operation size (0 = 16bit, 1 = 32bit)
    uint8_t  Granularity : 1;    // Granularity.
    uint8_t  BaseHigh;          // The last 8 bits of the base.
} __attribute__((packed));

struct __attribute__ ((aligned (8))) GDTPointer
{
	uint16_t Limit;
	uint64_t Base;
} __attribute__((packed));

struct IDTEntry
{
    uint16_t OffsetLow;           // The lower 16 bits of the address to jump to when this interrupt fires.
    uint16_t Selector;            // Kernel segment selector.
    uint8_t  IST;                // Interrupt Stack Table offset, zero for now
    uint8_t  Flags;               // Type and attributes, such as present bit, privilege level, etc.
    uint16_t OffsetMid;           // The middle 16 bits of the ISR address
    uint32_t OffsetHigh;          // The high 32 bits of the ISR address
    uint32_t Reserved;            // Reserved, must be zero
} __attribute__((packed));

extern "C" void ReloadSegments();

void InitGDT(uint8_t* target);
