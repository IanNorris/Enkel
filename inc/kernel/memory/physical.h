#pragma once

#include <stdint.h>
#include <stddef.h>

#define PAGE_BITS 12
#define PAGE_SIZE 4096
#define PAGE_MASK (~(PAGE_SIZE-1))

enum class EPhysicalState : uint8_t
{
    Free,
    Reserved,
    Used,
    Branch,
};

struct SPhysicalAddressMask
{
    EPhysicalState State : 2;
    uintptr_t Unused : 10;
    uintptr_t Address : 52;
};

struct SPhysicalState
{
    union
    {
        uintptr_t Address;
        SPhysicalAddressMask State;
    };

    void SetAddress(const uintptr_t AddressIn)
    {
        Address = AddressIn & PAGE_MASK;
    }
    uintptr_t GetAddress() const
    {
        return Address & PAGE_MASK;
    }
};

struct SPhysicalStateBranch : SPhysicalState
{
    SPhysicalState* Left;
    SPhysicalState* Right;
	uint64_t 		Remaining;
	uint64_t 		Largest;

    void SetAddress(const uintptr_t AddressIn)
    {
        Address = AddressIn & PAGE_MASK;
    }
    uintptr_t GetAddress() const
    {
        return Address & PAGE_MASK;
    }

} __attribute__((packed));

void PreparePhysicalFreeList(const uintptr_t HighestAddress);
void TagPhysicalRange(SPhysicalState** CurrentStateInOut, const uintptr_t LowAddress, const uintptr_t HighAddress, const EPhysicalState State, const uintptr_t OuterLowAddress = 0ULL, const uintptr_t OuterHighAddress = ~0ULL);
