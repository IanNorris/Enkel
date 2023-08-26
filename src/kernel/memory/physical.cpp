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
static_assert(sizeof(SPhysicalStateBranch) == 24, "Physical state branch not expected size");

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

void TagPhysicalRange(SPhysicalState** CurrentStateInOut, const uintptr_t LowAddress, const uintptr_t HighAddress, const EPhysicalState State, const uintptr_t OuterLowAddress, const uintptr_t OuterHighAddress)
{
    if (CurrentStateInOut == nullptr)
    {
        CurrentStateInOut = &PhysicalStateRoot;
    }

    SPhysicalState* CurrentState = *CurrentStateInOut;

    _ASSERTF(OuterLowAddress <= LowAddress, "Requested address is out of range");
    _ASSERTF(HighAddress <= OuterHighAddress, "Requested address is out of range");
    
    if (CurrentState->State.State == EPhysicalState::Branch)
    {
        uintptr_t Address = CurrentState->GetAddress();

        _ASSERTF(OuterLowAddress <= Address, "Requested address is out of range");
        _ASSERTF(Address < OuterHighAddress, "Requested address is out of range");

        if (LowAddress < Address && HighAddress <= Address)
        {
            //Take the left branch
            TagPhysicalRange(&((SPhysicalStateBranch*)CurrentState)->Left, LowAddress, HighAddress, State, OuterLowAddress, Address);
        }
        else if (LowAddress >= Address)
        {
            //Take the right branch
            TagPhysicalRange(&((SPhysicalStateBranch*)CurrentState)->Right, LowAddress, HighAddress, State, Address, OuterHighAddress);
        }
        else
        {
            //Do both
            TagPhysicalRange(&((SPhysicalStateBranch*)CurrentState)->Left, LowAddress, Address, State, OuterLowAddress, Address);
            TagPhysicalRange(&((SPhysicalStateBranch*)CurrentState)->Right, Address, HighAddress, State, Address, OuterHighAddress);
        }
    }
    else
    {
        uintptr_t NewOuterHighAddress = CurrentState->GetAddress();

        // We're on a leaf node. Check if we need to split it.
        if (!(LowAddress == OuterLowAddress && HighAddress == NewOuterHighAddress))
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
            NewBranch->Right->SetAddress(NewOuterHighAddress);
            NewBranch->Right->State.State = CurrentState->State.State;

            // Replace the current state with the new branch
            *CurrentStateInOut = NewBranch;

            // Now that the node is split, recurse on the appropriate child node
            if (LowAddress < MidPoint && HighAddress <= MidPoint)
            {
                TagPhysicalRange(&NewBranch->Left, LowAddress, HighAddress, State, OuterLowAddress, MidPoint);
            }
            else if (LowAddress >= MidPoint && HighAddress > MidPoint)
            {
                TagPhysicalRange(&NewBranch->Right, LowAddress, HighAddress, State, MidPoint, NewOuterHighAddress);
            }
            else
            {
                _ASSERTF(false, "Incorrect branch state");
            }
        }
        else
        {
            _ASSERTF(LowAddress == OuterLowAddress || HighAddress == OuterHighAddress, "Expected ranges to match.");
            _ASSERTF(CurrentState->State.State != EPhysicalState::Reserved, "Cannot modify reserved ranges once created.");

            CurrentState->State.State = State;
        }
    }
}
