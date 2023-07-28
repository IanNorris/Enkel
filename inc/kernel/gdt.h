#pragma once

enum SegmentType : uint8_t
{
    DATA_ReadWrite_NonAcc = 0x02,
    DATA_ReadWrite_Acc = 0x03,
    CODE_ExecuteOnly_NonAcc = 0x08,
    CODE_ExecuteRead_NonAcc = 0x0A,
    CODE_ExecuteOnly_Acc = 0x09,
    CODE_ExecuteRead_Acc = 0x0B
};

enum InterruptType : uint8_t
{
    InterruptGate = 0x8E,
    TrapGate = 0x8F
};

struct GDTEntry
{
    uint16_t LimitLow;                          //Lower 16bits of the limit
    uint16_t BaseLow;                           //Lower 16bits of the base
    uint8_t  BaseMiddle;                        //Next 8bits of the base
    uint8_t  SegmentType : 4;                   //Segment type (eg code/data)
    uint8_t  DescriptorType : 1;                //Descriptor type (0 == system, 1 = code or data) 
    uint8_t  DescriptorPrivilegeLevel : 2;      //Descriptor privilege level (DPL) - rings 0-3 (0==kernel)
    uint8_t  IsPresent : 1;                     //Is this descriptor present in memory?
    uint8_t  LimitHigh : 4; 			    	//Upper 4bits of the limit     
    uint8_t  IsAvailable : 1;                   //Is available for system use (always set to zero)
    uint8_t  LongModeSegment : 1;               //Is Long mode
    uint8_t  SegmentSizeMode : 1;  			    //Operation size
    uint8_t  Granularity : 1;                   //Grandularity of the segment. 0 = 1b, 1 = 4kb pages. When set the limit is shifted left by 12 bits and 1bits are filled in behind.
    uint8_t  BaseHigh;						    //Upper 8bits of the base 
} __attribute__((packed));

struct GDTPointer
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
