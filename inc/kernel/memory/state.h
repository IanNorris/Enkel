#pragma once

#include <stdint.h>
#include <stddef.h>
#include "memory/memory.h"

#define INVALID_ADDRESS (~0ULL)

#define PAGE_BITS 12
#define PAGE_SIZE 4096
#define PAGE_SIZE_2MB 0x200000
#define PAGE_MASK 0xFFFFFFFFF000

class MemoryState
{
public:

	enum class RangeState : uint8_t
	{
		Free,
		Reserved,
		Used,
		Branch,
	};

	struct AddressMask
	{
		RangeState State : 2;
		uintptr_t Unused : 10;
		uintptr_t Address : 52;
	};

	struct StateNode
	{
		union
		{
			uintptr_t Address;
			AddressMask State;
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

	struct BranchStateNode : StateNode
	{
		StateNode* 		Left;
		StateNode* 		Right;
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

	};

	void Init(int SystemIndex, const uint64_t HighestAddress);
	void InitDynamic();

	void AllocateNewLeafPage();
	void AllocateNewBranchPage();

	void TagRange(const uintptr_t LowAddress, const uintptr_t HighAddress, const RangeState State)
	{
		StateRoot = TagRangeInternal(StateRoot, LowAddress, HighAddress, State, 0ULL, HighestAddress);
	}

	uintptr_t FindMinimumSizeFreeBlock(uint64_t MinSize)
	{
		return FindMinimumSizeFreeBlockInternal(StateRoot, MinSize, 0ULL, HighestAddress);
	}

	RangeState GetPageState(const uint64_t Address);

private:

	StateNode* TagRangeInternal(StateNode* CurrentState, const uintptr_t LowAddress, const uintptr_t HighAddress, const RangeState State, const uintptr_t OuterLowAddress, const uintptr_t OuterHighAddress);
	uintptr_t FindMinimumSizeFreeBlockInternal(StateNode* CurrentState, uint64_t MinSize, const uintptr_t OuterLowAddress, const uintptr_t OuterHighAddress);
	RangeState GetPageStateInternal(StateNode* CurrentState, const uint64_t Address);

	BranchStateNode* GetFreeBranchState()
	{
		if(StateBranchFreeHead == nullptr)
		{
			StateBranchFreeHead = NextStateBranchFreeHead;
			AllocateNewBranchPage();
		}

		BranchStateNode* Result = StateBranchFreeHead;
		StateBranchFreeHead = (BranchStateNode*)StateBranchFreeHead->Address;

		UsedBranches++;

		return Result;
	}

	void FreeStateBranch(BranchStateNode* State)
	{
		memset(State, 0, sizeof(BranchStateNode));

		State->Address = (uintptr_t)StateBranchFreeHead;
		StateBranchFreeHead = State;

		UsedBranches--;
	}

	StateNode* GetFreeLeafState()
	{
		if(StateLeafFreeHead == nullptr)
		{
			StateLeafFreeHead = NextStateLeafFreeHead;
			AllocateNewLeafPage();
		}

		StateNode* Result = StateLeafFreeHead;
		StateLeafFreeHead = (StateNode*)StateLeafFreeHead->Address;

		UsedLeaves++;

		return Result;
	}

	void FreeLeafState(StateNode* State)
	{
		memset(State, 0, sizeof(StateNode));

		State->Address = (uintptr_t)StateLeafFreeHead;
		StateLeafFreeHead = State;

		UsedLeaves--;
	}

	uint64_t GetRemaining(StateNode* Node, const uintptr_t OuterLowAddress, const uintptr_t OuterHighAddress)
	{
		if (Node->State.State == RangeState::Branch)
		{
			BranchStateNode* BranchNode = (BranchStateNode*)Node;
			return BranchNode->Remaining;
		}
		else if(Node->State.State == RangeState::Free)
		{
			_ASSERTF(OuterHighAddress - OuterLowAddress > 0, "Found empty node");
			return OuterHighAddress - OuterLowAddress;
		}
		else
		{
			return 0;
		}
	}

	uint64_t GetLargestFree(StateNode* Node, const uintptr_t OuterLowAddress, const uintptr_t OuterHighAddress)
	{
		if (Node->State.State == RangeState::Branch)
		{
			BranchStateNode* BranchNode = (BranchStateNode*)Node;
			return BranchNode->Largest;
		}
		else if(Node->State.State == RangeState::Free)
		{
			_ASSERTF(OuterHighAddress - OuterLowAddress > 0, "Found empty node");
			return OuterHighAddress - OuterLowAddress;
		}
		else
		{
			return 0;
		}
	}

	//We call this after tagging a block in a descendant to update the current state of this node based on the new descendants.
	StateNode* UpdateNode(BranchStateNode* BranchState, const uintptr_t OuterLowAddress, const uintptr_t OuterHighAddress)
	{
		if(BranchState->Right->State.State != RangeState::Branch && BranchState->Left->State.State == BranchState->Right->State.State)
		{
			StateNode* Left = BranchState->Left;
			StateNode* Right = BranchState->Right;

			StateNode* NewNode = GetFreeLeafState();
			NewNode->SetAddress(OuterHighAddress);
			NewNode->State.State = BranchState->Left->State.State;

			FreeStateBranch(BranchState);
			FreeLeafState(Left);
			FreeLeafState(Right);

			return NewNode;
		}

		uintptr_t Mid = BranchState->GetAddress();
		uint64_t LargestLeft = GetLargestFree(BranchState->Left, OuterLowAddress, Mid);
		uint64_t LargestRight = GetLargestFree(BranchState->Right, Mid, OuterHighAddress);

		_ASSERTF(OuterHighAddress - OuterLowAddress > 0, "Encountered empty block");

		uint64_t RemainingLeft = GetRemaining(BranchState->Left, OuterLowAddress, Mid);
		uint64_t RemainingRight = GetRemaining(BranchState->Right, Mid, OuterHighAddress);

		BranchState->Largest = LargestLeft > LargestRight ? LargestLeft : LargestRight;
		BranchState->Remaining = RemainingLeft + RemainingRight;

		return BranchState;
	}

	const static int InitialLeafTableEntries = 4096;
	const static int InitialBranchTableEntries = 4096;

	StateNode* StateRoot;
	StateNode* StateLeafFreeHead;
	BranchStateNode* StateBranchFreeHead;

	StateNode InitialLeafEntries[InitialLeafTableEntries];
	BranchStateNode InitialBranchEntries[InitialBranchTableEntries];

	StateNode* NextStateLeafFreeHead;
	BranchStateNode* NextStateBranchFreeHead;

	uint64_t HighestAddress;
	uint64_t UsedBranches;
	uint64_t UsedLeaves;

	int SystemIndex; //This is for identifying what system were in when debugging

	static_assert(sizeof(StateNode) == 8, "Leaf state not expected size");
	static_assert(sizeof(BranchStateNode) == 40, "Branch state not expected size");
};
