#include "kernel/init/init.h"
#include "kernel/init/bootload.h"
#include "kernel/init/long_mode.h"
#include "kernel/console/console.h"
#include "memory/memory.h"
#include "kernel/memory/state.h"
#include "common/string.h"

MemoryState PhysicalMemoryState;
MemoryState VirtualMemoryState;

//Summary of the system
//---------------------
// We track memory with a tree structure. This tree structure is made
// up of two types of nodes - branches and leaves. Branches extend leaves
// with Left and Right pointers. To allocate a specific block of memory
// you need to walk the tree structure. This is achieved by first checking the
// node type.
//
// If you encounter a branch node, the Address pointer contains the address that
// divides the Left and Right branches. The Left branch will contain the range from
// the parent lower bound to Address (not inclusive, but we only track pages so we 
// don't care), and the Right node will contain from Address to the parent upper bound.
// Therefore you need to pick the branch that contains the range you need.
//
// If you encounter a leaf node (ie anything except a branch), the Address pointer
// now contains the upper bound of the allocation from the parent's lower bound.

void MemoryState::Init(const uint64_t highestAddress)
{
	StateRoot = nullptr;
	StateLeafFreeHead = nullptr;
	StateBranchFreeHead = nullptr;
	HighestAddress = highestAddress;

	static_assert(sizeof(InitialLeafEntries) == sizeof(StateNode) * 4096, "Table size doesn't match expected");

	memset(InitialLeafEntries, 0, sizeof(InitialLeafEntries));
    memset(InitialBranchEntries, 0, sizeof(InitialBranchEntries));

	// Link all entries together to form the free list
    for (int i = InitialLeafTableEntries - 1; i >= 0; i--)
    {
        InitialLeafEntries[i].Address = (uintptr_t)StateLeafFreeHead;
        StateLeafFreeHead = &InitialLeafEntries[i];
    }

    for (int i = InitialBranchTableEntries - 1; i >= 0; i--)
    {
        InitialBranchEntries[i].Address = (uintptr_t)StateBranchFreeHead;
        StateBranchFreeHead = &InitialBranchEntries[i];
    }

   // Set the root to the first entry
    StateRoot = GetFreeBranchState();
    StateRoot->SetAddress(highestAddress);
    StateRoot->State.State = RangeState::Free;
}

MemoryState::StateNode* MemoryState::TagRangeInternal(StateNode* CurrentState, const uintptr_t LowAddress, const uintptr_t HighAddress, const RangeState State, const uintptr_t OuterLowAddress, const uintptr_t OuterHighAddress)
{
	_ASSERTF(OuterHighAddress - OuterLowAddress > 0, "About to create an empty block");

    _ASSERTF(OuterLowAddress <= LowAddress, "Requested address is out of range");
    _ASSERTF(HighAddress <= OuterHighAddress, "Requested address is out of range");

	uintptr_t Address = CurrentState->GetAddress();

    if (CurrentState->State.State == RangeState::Branch)
    {
		BranchStateNode* BranchState = (BranchStateNode*)CurrentState;

        _ASSERTF(OuterLowAddress <= Address, "Requested address is out of range");
        _ASSERTF(Address < OuterHighAddress, "Requested address is out of range");

        if (LowAddress < Address && HighAddress <= Address)
        {
            //Take the left branch
            BranchState->Left = TagRangeInternal(BranchState->Left, LowAddress, HighAddress, State, OuterLowAddress, Address);
        }
        else if (LowAddress >= Address)
        {
            //Take the right branch
            BranchState->Right = TagRangeInternal(BranchState->Right, LowAddress, HighAddress, State, Address, OuterHighAddress);
        }
        else
        {
            //Do both
            BranchState->Left = TagRangeInternal(BranchState->Left, LowAddress, Address, State, OuterLowAddress, Address);
            BranchState->Right = TagRangeInternal(BranchState->Right, Address, HighAddress, State, Address, OuterHighAddress);
        }

		//We may no longer actually be a branch state now if one of the nodes above merged back to a leaf
		if (CurrentState->State.State == RangeState::Branch)
    	{
			StateNode* NewNode = UpdateNode(BranchState, OuterLowAddress, OuterHighAddress);
			if(CurrentState != NewNode)
			{
				CurrentState = NewNode;
			}
		}
    }
    else
    {
		_ASSERTF(Address >= LowAddress && Address >= HighAddress, "Address out of bounds");

        // We're on a leaf node. Check if we need to split it.
        if (!(LowAddress == OuterLowAddress && HighAddress == Address))
        {
			//If the range we want is the same as what's already tagged
			// and is wholly contained in our current block, we can just do nothing.
			if(LowAddress >= OuterLowAddress && HighAddress <= Address && CurrentState->State.State == State)
			{
				return CurrentState;
			}

            //If our lower bound doesn't touch the outer lower bound, then we treat the new low address as the mid point.
            //However if that's not true then our high address is the mid point
            uint64_t MidPoint = LowAddress == OuterLowAddress ? HighAddress : LowAddress;

            // Split the node into a branch with two children
            BranchStateNode* NewBranch = GetFreeBranchState();
            NewBranch->SetAddress(MidPoint); //Address in this scenario is the mid point
            NewBranch->State.State = RangeState::Branch;

			_ASSERTF(OuterLowAddress < MidPoint, "About to create zero block");
			StateNode* NewLeft = GetFreeLeafState();
            NewBranch->Left = NewLeft;
            NewLeft->SetAddress(MidPoint);
            NewLeft->State.State = CurrentState->State.State;

			if(MidPoint != NewLeft->GetAddress())
			{
				char16_t buffer[16];
				ConsolePrint(u"MidPoint: ");
				witoabuf(buffer, MidPoint, 16);
				ConsolePrint(buffer);

				ConsolePrint(u"\nNewLeft Address: ");
				witoabuf(buffer, NewLeft->GetAddress(), 16);
				ConsolePrint(buffer);

				ConsolePrint(u"\n");
			}

			_ASSERTF(MidPoint == NewLeft->GetAddress(), "Mismatch on expected block bounds");

			_ASSERTF(Address == OuterHighAddress || (OuterHighAddress & PAGE_MASK) == (~0ULL & PAGE_MASK), "Right block should touch the outer bound");
			StateNode* NewRight = GetFreeLeafState();
            NewBranch->Right = NewRight;
            NewRight->SetAddress(Address);
            NewRight->State.State = CurrentState->State.State;

			//Free the old node
			FreeLeafState(CurrentState);

			CurrentState = NewBranch;

			_ASSERTF(MidPoint == NewBranch->Left->GetAddress(), "Mismatch on expected block bounds");

            // Now that the node is split, recurse on the appropriate child node
            if (LowAddress < MidPoint && HighAddress <= MidPoint)
            {
				_ASSERTF(OuterLowAddress < MidPoint, "About to create zero block");
				_ASSERTF(MidPoint == NewBranch->Left->GetAddress(), "Mismatch on expected block bounds");
                NewBranch->Left = TagRangeInternal(NewBranch->Left, LowAddress, HighAddress, State, OuterLowAddress, MidPoint);
            }
            else if (LowAddress >= MidPoint && HighAddress > MidPoint)
            {
				_ASSERTF(MidPoint < Address, "About to create zero block");
                NewBranch->Right = TagRangeInternal(NewBranch->Right, LowAddress, HighAddress, State, MidPoint, Address);
            }
            else
            {
                _ASSERTF(false, "Incorrect branch state");
            }

			StateNode* NewNode = UpdateNode(NewBranch, OuterLowAddress, OuterHighAddress);
			_ASSERTF(NewNode == NewBranch, "Unexpected node merge");
        }
        else
        {
            _ASSERTF(LowAddress == OuterLowAddress || HighAddress == OuterHighAddress, "Expected ranges to match.");
            //_ASSERTF(CurrentState->State.State != RangeState::Reserved || State == RangeState::Reserved, "Cannot modify reserved ranges once created.");

            CurrentState->State.State = State;
        }
    }

	return CurrentState;
}

uintptr_t MemoryState::FindMinimumSizeFreeBlockInternal(StateNode* CurrentState, uint64_t MinSize, const uintptr_t OuterLowAddress, const uintptr_t OuterHighAddress)
{
	uintptr_t Address = CurrentState->GetAddress();

    if (CurrentState->State.State == RangeState::Branch)
    {
		BranchStateNode* BranchState = (BranchStateNode*)CurrentState;

		uintptr_t Mid = BranchState->GetAddress();
		uint64_t LargestLeft = GetLargestFree(BranchState->Left, OuterLowAddress, Mid);
		uint64_t LargestRight = GetLargestFree(BranchState->Right, Mid, OuterHighAddress);

		if(LargestLeft >= MinSize)
		{
			uintptr_t Candidate = FindMinimumSizeFreeBlockInternal(BranchState->Left, MinSize, OuterLowAddress, Mid);
			if(Candidate != 0)
			{
				return Candidate;
			}
		}

		if(LargestRight >= MinSize)
		{
			uintptr_t Candidate = FindMinimumSizeFreeBlockInternal(BranchState->Right, MinSize, Mid, OuterHighAddress);
			if(Candidate != 0)
			{
				return Candidate;
			}
		}

		return 0;
    }
	else if (CurrentState->State.State == RangeState::Free)
    {
		uint64_t Largest = GetLargestFree(CurrentState, OuterLowAddress, Address);
		if(Largest >= MinSize)
		{
			return OuterLowAddress;
		}
	}
    
	return 0;
}
