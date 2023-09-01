#include "kernel/init/init.h"
#include "kernel/init/bootload.h"
#include "kernel/init/long_mode.h"
#include "kernel/console/console.h"
#include "memory/memory.h"
#include "kernel/memory/physical.h"
#include "common/string.h"

SPhysicalState* PhysicalStateRoot = nullptr;

SPhysicalState* PhysicalStateFreeHead = nullptr;
SPhysicalStateBranch* PhysicalStateBranchFreeHead = nullptr;

#define INITIAL_PHYSICAL_ENTRIES 4096
#define INITIAL_PHYSICAL_BRANCH_ENTRIES 4096
SPhysicalState InitialPhysicalEntries[INITIAL_PHYSICAL_ENTRIES];
SPhysicalStateBranch InitialPhysicalBranchEntries[INITIAL_PHYSICAL_BRANCH_ENTRIES];

static_assert(sizeof(SPhysicalState) == 8, "Physical state not expected size");
static_assert(sizeof(SPhysicalStateBranch) == 40, "Physical state branch not expected size");

//Summary of the system
//---------------------
// We track physical memory with a tree structure. This tree structure is made
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

extern SPhysicalStateBranch* GetFreePhysicalStateBranch();

void PreparePhysicalFreeList(const uintptr_t HighestAddress)
{
    memset(InitialPhysicalEntries, 0, sizeof(InitialPhysicalEntries));
    memset(InitialPhysicalBranchEntries, 0, sizeof(InitialPhysicalBranchEntries));

    PhysicalStateFreeHead = nullptr;
    PhysicalStateBranchFreeHead = nullptr;

    // Link all entries together to form the free list
    for (int i = INITIAL_PHYSICAL_ENTRIES - 1; i >= 0; i--)
    {
        InitialPhysicalEntries[i].Address = (uintptr_t)PhysicalStateFreeHead;
        PhysicalStateFreeHead = &InitialPhysicalEntries[i];
    }

    for (int i = INITIAL_PHYSICAL_BRANCH_ENTRIES - 1; i >= 0; i--)
    {
        InitialPhysicalBranchEntries[i].Address = (uintptr_t)PhysicalStateBranchFreeHead;
        PhysicalStateBranchFreeHead = &InitialPhysicalBranchEntries[i];
    }

    // Set the root to the first entry
    PhysicalStateRoot = GetFreePhysicalStateBranch();
    PhysicalStateRoot->SetAddress(HighestAddress);
    PhysicalStateRoot->State.State = EPhysicalState::Free;
}

SPhysicalStateBranch* GetFreePhysicalStateBranch()
{
    SPhysicalStateBranch* Result = PhysicalStateBranchFreeHead;
    PhysicalStateBranchFreeHead = (SPhysicalStateBranch*)PhysicalStateBranchFreeHead->Address;

    return Result;
}

void FreePhysicalStateBranch(SPhysicalStateBranch* State)
{
    memset(State, 0, sizeof(SPhysicalStateBranch));

    State->Address = (uintptr_t)PhysicalStateBranchFreeHead;
    PhysicalStateBranchFreeHead = State;
}

SPhysicalState* GetFreePhysicalState()
{
    SPhysicalState* Result = PhysicalStateFreeHead;
    PhysicalStateFreeHead = (SPhysicalState*)PhysicalStateFreeHead->Address;

    return Result;
}

void FreePhysicalState(SPhysicalState* State)
{
    memset(State, 0, sizeof(SPhysicalState));

    State->Address = (uintptr_t)PhysicalStateFreeHead;
    PhysicalStateFreeHead = State;
}

uint64_t GetRemaining(SPhysicalState* Node, const uintptr_t OuterLowAddress, const uintptr_t OuterHighAddress)
{
	if (Node->State.State == EPhysicalState::Branch)
    {
		SPhysicalStateBranch* BranchNode = (SPhysicalStateBranch*)Node;
		return BranchNode->Remaining;
	}
	else if(Node->State.State == EPhysicalState::Free)
	{
		_ASSERTF(OuterHighAddress - OuterLowAddress > 0, "Found empty node");
		return OuterHighAddress - OuterLowAddress;
	}
	else
	{
		return 0;
	}
}

uint64_t GetLargestFree(SPhysicalState* Node, const uintptr_t OuterLowAddress, const uintptr_t OuterHighAddress)
{
	if (Node->State.State == EPhysicalState::Branch)
    {
		SPhysicalStateBranch* BranchNode = (SPhysicalStateBranch*)Node;
		return BranchNode->Largest;
	}
	else if(Node->State.State == EPhysicalState::Free)
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
void UpdateNode(SPhysicalStateBranch* BranchState, const uintptr_t OuterLowAddress, const uintptr_t OuterHighAddress)
{
	uintptr_t Mid = BranchState->GetAddress();
	uint64_t LargestLeft = GetLargestFree(BranchState->Left, OuterLowAddress, Mid);
	uint64_t LargestRight = GetLargestFree(BranchState->Right, Mid, OuterHighAddress);

	_ASSERTF(OuterHighAddress - OuterLowAddress > 0, "About to create an empty block");

	uint64_t RemainingLeft = GetRemaining(BranchState->Left, OuterLowAddress, Mid);
	uint64_t RemainingRight = GetRemaining(BranchState->Right, Mid, OuterHighAddress);

	BranchState->Largest = LargestLeft > LargestRight ? LargestLeft : LargestRight;
	BranchState->Remaining = RemainingLeft + RemainingRight;
}

void TagPhysicalRange(SPhysicalState** CurrentStateInOut, const uintptr_t LowAddress, const uintptr_t HighAddress, const EPhysicalState State, const uintptr_t OuterLowAddress, const uintptr_t OuterHighAddress)
{
	_ASSERTF(OuterHighAddress - OuterLowAddress > 0, "About to create an empty block");

    SPhysicalState* CurrentState = *CurrentStateInOut;

    _ASSERTF(OuterLowAddress <= LowAddress, "Requested address is out of range");
    _ASSERTF(HighAddress <= OuterHighAddress, "Requested address is out of range");

	uintptr_t Address = CurrentState->GetAddress();

    if (CurrentState->State.State == EPhysicalState::Branch)
    {
		SPhysicalStateBranch* BranchState = (SPhysicalStateBranch*)CurrentState;

        _ASSERTF(OuterLowAddress <= Address, "Requested address is out of range");
        _ASSERTF(Address < OuterHighAddress, "Requested address is out of range");

        if (LowAddress < Address && HighAddress <= Address)
        {
            //Take the left branch
            TagPhysicalRange(&BranchState->Left, LowAddress, HighAddress, State, OuterLowAddress, Address);
        }
        else if (LowAddress >= Address)
        {
            //Take the right branch
            TagPhysicalRange(&BranchState->Right, LowAddress, HighAddress, State, Address, OuterHighAddress);
        }
        else
        {
            //Do both
            TagPhysicalRange(&BranchState->Left, LowAddress, Address, State, OuterLowAddress, Address);
            TagPhysicalRange(&BranchState->Right, Address, HighAddress, State, Address, OuterHighAddress);
        }

		UpdateNode(BranchState, OuterLowAddress, OuterHighAddress);
    }
    else
    {
        // We're on a leaf node. Check if we need to split it.
        if (!(LowAddress == OuterLowAddress && HighAddress == Address))
        {
            //If our lower bound doesn't touch the outer lower bound, then we treat the new low address as the mid point.
            //However if that's not true then our high address is the mid point
            uint64_t MidPoint = LowAddress == OuterLowAddress ? HighAddress : LowAddress;

            // Split the node into a branch with two children
            SPhysicalStateBranch* NewBranch = GetFreePhysicalStateBranch();
            NewBranch->SetAddress(MidPoint); //Address in this scenario is the mid point
            NewBranch->State.State = EPhysicalState::Branch;

            NewBranch->Left = GetFreePhysicalState();
            NewBranch->Left->SetAddress(MidPoint);
            NewBranch->Left->State.State = CurrentState->State.State;

            NewBranch->Right = GetFreePhysicalState();
            NewBranch->Right->SetAddress(Address);
            NewBranch->Right->State.State = CurrentState->State.State;

            // Replace the current state with the new branch
            *CurrentStateInOut = NewBranch;

            // Now that the node is split, recurse on the appropriate child node
            if (LowAddress < MidPoint && HighAddress <= MidPoint)
            {
				_ASSERTF(OuterLowAddress < MidPoint, "About to create zero block");
                TagPhysicalRange(&NewBranch->Left, LowAddress, HighAddress, State, OuterLowAddress, MidPoint);
            }
            else if (LowAddress >= MidPoint && HighAddress > MidPoint)
            {
				_ASSERTF(MidPoint < Address, "About to create zero block");
                TagPhysicalRange(&NewBranch->Right, LowAddress, HighAddress, State, MidPoint, Address);
            }
            else
            {
                _ASSERTF(false, "Incorrect branch state");
            }

			UpdateNode(NewBranch, OuterLowAddress, OuterHighAddress);
        }
        else
        {
            _ASSERTF(LowAddress == OuterLowAddress || HighAddress == OuterHighAddress, "Expected ranges to match.");
            _ASSERTF(CurrentState->State.State != EPhysicalState::Reserved, "Cannot modify reserved ranges once created.");

            CurrentState->State.State = State;
        }
    }
}

uintptr_t FindMinimumSizeFreeBlock(SPhysicalState* CurrentState, uint64_t MinSize, const uintptr_t OuterLowAddress, const uintptr_t OuterHighAddress)
{
	uintptr_t Address = CurrentState->GetAddress();

    if (CurrentState->State.State == EPhysicalState::Branch)
    {
		SPhysicalStateBranch* BranchState = (SPhysicalStateBranch*)CurrentState;

		uintptr_t Mid = BranchState->GetAddress();
		uint64_t LargestLeft = GetLargestFree(BranchState->Left, OuterLowAddress, Mid);
		uint64_t LargestRight = GetLargestFree(BranchState->Right, Mid, OuterHighAddress);

		if(LargestLeft >= MinSize)
		{
			uintptr_t Candidate = FindMinimumSizeFreeBlock(BranchState->Left, MinSize, OuterLowAddress, Mid);
			if(Candidate != 0)
			{
				return Candidate;
			}
		}

		if(LargestRight >= MinSize)
		{
			uintptr_t Candidate = FindMinimumSizeFreeBlock(BranchState->Right, MinSize, Mid, OuterHighAddress);
			if(Candidate != 0)
			{
				return Candidate;
			}
		}

		return 0;
    }
	else if (CurrentState->State.State == EPhysicalState::Free)
    {
		uint64_t Largest = GetLargestFree(CurrentState, OuterLowAddress, Address);
		if(Largest >= MinSize)
		{
			return OuterLowAddress;
		}
	}
    else
    {
        return 0;
    }
}
