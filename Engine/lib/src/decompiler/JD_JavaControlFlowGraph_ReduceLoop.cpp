#include <jdc/decompiler/JD_JavaControlFlowGraph.hpp>
#include <jdc/decompiler/JD_JavaControlFlowGraph_JavaLoop.hpp>
#include <jdc/decompiler/JD_JavaMethod.hpp>
#include <jdc/decompiler/JD_JavaClass.hpp>
#include <jdc/decompiler/JD_JavaSpace.hpp>
#include <xel/String.hpp>
#include <algorithm>
#include <sstream>

using namespace std;
using namespace xel;

namespace jdc
{

    static std::vector<xBitSet> BuildDominatorIndexes(xJavaBlockPtrList & BlockPtrList)
    {
        auto Length = BlockPtrList.size();
        auto ArrayOfDominatorIndexes = std::vector<xBitSet>(Length);
        ArrayOfDominatorIndexes[0] = xBitSet(Length);
        ArrayOfDominatorIndexes[0][0] = true;

        for (size_t i = 1; i < Length; ++i) {
            auto & Item = ArrayOfDominatorIndexes[i];
            Item = xBitSet(Length, true);
        }

        auto Change = false;
        do {
            for (size_t Index = 1; Index < Length; ++Index) {
                auto BlockPtr = BlockPtrList[Index];
                assert(BlockPtr->Index == Index);

                auto & DominatorIndexes = ArrayOfDominatorIndexes[Index];
                auto   Backup = DominatorIndexes;
                for (auto PredecessorBlockPtr : BlockPtr->Predecessors) {
                    auto & PredecessorDominatorIndexes = ArrayOfDominatorIndexes[PredecessorBlockPtr->Index];
                    for (size_t i = 0; i < DominatorIndexes.size(); ++i) {
                        DominatorIndexes[i] = DominatorIndexes[i] & PredecessorDominatorIndexes[i];
                    }
                }
                DominatorIndexes[Index] = true;

                Change |= DominatorIndexes != Backup;
            }
        } while(Steal(Change));

        for (auto & Item : ArrayOfDominatorIndexes) {
            X_DEBUG_PRINTF("%s\n", ToString(Item).c_str());
        }

        return ArrayOfDominatorIndexes;
    }

    static void RecursiveForwardSearchLoopMemberIndexes(xBitSet & Visited, const xBitSet & SearchZoneIndexes, xJavaBlock * CurrentBlockPtr, size_t MaxOffset) {
        if ((!CurrentBlockPtr->Type & (
                xJavaBlock::TYPE_END | xJavaBlock::TYPE_LOOP_START | xJavaBlock::TYPE_LOOP_CONTINUE | xJavaBlock::TYPE_LOOP_END | xJavaBlock::TYPE_SWITCH_BREAK))
            && (!Visited[CurrentBlockPtr->Index])
            && (SearchZoneIndexes[CurrentBlockPtr->Index])
            && (CurrentBlockPtr->FromOffset <= MaxOffset)) {

            Visited[CurrentBlockPtr->Index] = true;
            RecursiveForwardSearchLoopMemberIndexes(Visited, SearchZoneIndexes, CurrentBlockPtr->NextBlockPtr, MaxOffset);
            RecursiveForwardSearchLoopMemberIndexes(Visited, SearchZoneIndexes, CurrentBlockPtr->BranchBlockPtr, MaxOffset);

            for (auto & SwitchCase : CurrentBlockPtr->SwitchCases) {
                RecursiveForwardSearchLoopMemberIndexes(Visited, SearchZoneIndexes, SwitchCase.BlockPtr, MaxOffset);
            }

            for (auto & ExceptionHandler : CurrentBlockPtr->ExceptionHandlers) {
                RecursiveForwardSearchLoopMemberIndexes(Visited, SearchZoneIndexes, ExceptionHandler.BlockPtr, MaxOffset);
            }

            if (CurrentBlockPtr->Type == xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR) {
                Visited[CurrentBlockPtr->NextBlockPtr->Index] = true;
            }
        }
    }

    static void RecursiveForwardSearchLoopMemberIndexes(xBitSet & Visited, const xBitSet & SearchZoneIndexes, xJavaBlock * CurrentBlockPtr, xJavaBlock *  TargetBlockPtr) {
        if (!(CurrentBlockPtr->Type & xJavaBlock::GROUP_END)
            && (!Visited[CurrentBlockPtr->Index])
            && (SearchZoneIndexes[CurrentBlockPtr->Index])) {

            Visited[CurrentBlockPtr->Index] = true;

            if (CurrentBlockPtr != TargetBlockPtr) {
                RecursiveForwardSearchLoopMemberIndexes(Visited, SearchZoneIndexes, CurrentBlockPtr->NextBlockPtr, TargetBlockPtr);
                RecursiveForwardSearchLoopMemberIndexes(Visited, SearchZoneIndexes, CurrentBlockPtr->BranchBlockPtr, TargetBlockPtr);

                for (auto & SwitchCase : CurrentBlockPtr->SwitchCases) {
                    RecursiveForwardSearchLoopMemberIndexes(Visited, SearchZoneIndexes, SwitchCase.BlockPtr, TargetBlockPtr);
                }

                for (auto & ExceptionHandler : CurrentBlockPtr->ExceptionHandlers) {
                    RecursiveForwardSearchLoopMemberIndexes(Visited, SearchZoneIndexes, ExceptionHandler.BlockPtr, TargetBlockPtr);
                }

                if (CurrentBlockPtr->Type == xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR) {
                    Visited[CurrentBlockPtr->NextBlockPtr->Index];
                }
            }
        }
    }

    static void RecursiveBackwardSearchLoopMemberIndexes(xBitSet & Visited, xJavaBlock * CurrentBlockPtr, xJavaBlock * StartBlockPtr)
    {
        if (!Visited[CurrentBlockPtr->Index]) {
            Visited[CurrentBlockPtr->Index] = true;
            if (CurrentBlockPtr != StartBlockPtr) {
                for (xJavaBlock * PredecessorBlockPtr : CurrentBlockPtr->Predecessors) {
                    RecursiveBackwardSearchLoopMemberIndexes(Visited, PredecessorBlockPtr, StartBlockPtr);
                }
            }
        }
    }

    static bool InSearchZone(xJavaBlock * BlockPtr, const xBitSet & SearchZoneIndexes) {
        return
            (BlockPtr->Type & (xJavaBlock::TYPE_END | xJavaBlock::TYPE_RETURN | xJavaBlock::TYPE_RET | xJavaBlock::TYPE_LOOP_END | xJavaBlock::TYPE_LOOP_START | xJavaBlock::TYPE_INFINITE_GOTO | xJavaBlock::TYPE_JUMP))
            || SearchZoneIndexes[BlockPtr->Index];
    }

    static bool PredecessorsInSearchZone(xJavaBlock * BlockPtr, const xBitSet & SearchZoneIndexes) {
        auto & Predecessors = BlockPtr->Predecessors;

        for (auto & PredecessorPtr : Predecessors) {
            if (!InSearchZone(PredecessorPtr, SearchZoneIndexes)) {
                return false;
            }
        }
        return true;
    }

    static bool RecursiveForwardSearchLastLoopMemberIndexes(const xJavaBlockPtrSet & Members, xBitSet & SearchZoneIndexes, xJavaBlockPtrSet Set, xJavaBlock * CurrentBlockPtr, xJavaBlock * EndBlockPtr) {
        if ((CurrentBlockPtr == EndBlockPtr) || Members.find(CurrentBlockPtr) != Members.end() || Set.find(CurrentBlockPtr) != Set.end()) {
            return true;
        } else if (CurrentBlockPtr->Type & xJavaBlock::GROUP_SINGLE_SUCCESSOR) {
            if (!InSearchZone(CurrentBlockPtr->NextBlockPtr, SearchZoneIndexes) || !PredecessorsInSearchZone(CurrentBlockPtr, SearchZoneIndexes)) {
                SearchZoneIndexes[CurrentBlockPtr->Index] = false;
                return true;
            } else {
                Set.insert(CurrentBlockPtr);
                return RecursiveForwardSearchLastLoopMemberIndexes(Members, SearchZoneIndexes, Set, CurrentBlockPtr->NextBlockPtr, EndBlockPtr);
            }
        } else if (CurrentBlockPtr->Type == xJavaBlock::TYPE_CONDITIONAL_BRANCH) {
            if (!InSearchZone(CurrentBlockPtr->NextBlockPtr, SearchZoneIndexes) || !InSearchZone(CurrentBlockPtr->BranchBlockPtr, SearchZoneIndexes) || !PredecessorsInSearchZone(CurrentBlockPtr, SearchZoneIndexes)) {
                SearchZoneIndexes[CurrentBlockPtr->Index] = false;
                return true;
            } else {
                Set.insert(CurrentBlockPtr);
                return RecursiveForwardSearchLastLoopMemberIndexes(Members, SearchZoneIndexes, Set, CurrentBlockPtr->NextBlockPtr, EndBlockPtr) |
                       RecursiveForwardSearchLastLoopMemberIndexes(Members, SearchZoneIndexes, Set, CurrentBlockPtr->BranchBlockPtr, EndBlockPtr);
            }
        } else if (CurrentBlockPtr->Type & xJavaBlock::GROUP_END) {
            if (!PredecessorsInSearchZone(CurrentBlockPtr, SearchZoneIndexes)) {
                if (!CurrentBlockPtr->IsTheEnd()) {
                    SearchZoneIndexes[CurrentBlockPtr->Index] = false;
                }
            } else {
                Set.insert(CurrentBlockPtr);
            }
            return true;
        }

        return false;
    }

    static xBitSet SearchLoopMemberIndexes(size_t Length, xBitSet & MemberIndexes, xJavaBlock * CurrentBlockPtr, xJavaBlock * StartBlockPtr)
    {
        // assert(Length == MemberIndexes.size()); // TODO: maybe remove Length argument

        xBitSet Visited = xBitSet(Length);
        RecursiveBackwardSearchLoopMemberIndexes(Visited, CurrentBlockPtr, StartBlockPtr);
        if (MemberIndexes.empty()) {
            return Visited;
        }
        /* @brief MemberIndexes.or(Visited); */
        assert(MemberIndexes.size() == Visited.size());
        for (size_t i = 0; i < MemberIndexes.size(); ++i) {
            MemberIndexes[i] = MemberIndexes[i] | Visited[i];
        }
        return MemberIndexes;
    }

    static size_t CheckThrowBlockOffset(xJavaBlock * BlockPtr)
    {
        size_t Offset = BlockPtr->FromOffset;
        auto Watchdog = xBitSet();

        while (!(BlockPtr->Type & xJavaBlock::GROUP_END) && !Watchdog[BlockPtr->Index]) {
            Watchdog[BlockPtr->Index] = true;
            BlockPtr = BlockPtr->NextBlockPtr;
        }
        if (BlockPtr->Type == xJavaBlock::TYPE_THROW) {
            return BlockPtr->FromOffset;
        }
        return Offset;
    }

    static size_t CheckSynchronizedBlockOffset(xJavaBlock * BlockPtr)
    {
        if ((BlockPtr->NextBlockPtr->Type == xJavaBlock::TYPE_TRY_DECLARATION) && (BlockPtr->GetLastOpCode() == 194)) { // MONITORENTER
            return CheckThrowBlockOffset(BlockPtr->NextBlockPtr->ExceptionHandlers[0].BlockPtr);
        }

        return BlockPtr->FromOffset;
    }

    static size_t CheckMaxOffset(xJavaBlock * BlockPtr) {
        size_t MaxOffset = BlockPtr->FromOffset;
        size_t Offset = 0;

        if (BlockPtr->Type == xJavaBlock::TYPE_TRY_DECLARATION) {
            for (auto & ExceptionHandler : BlockPtr->ExceptionHandlers) {
                if (ExceptionHandler.FixedCatchTypeName.empty()) {
                    // Search throw block
                    Offset = CheckThrowBlockOffset(ExceptionHandler.BlockPtr);
                } else {
                    Offset = CheckSynchronizedBlockOffset(ExceptionHandler.BlockPtr);
                }
                if (MaxOffset < Offset) {
                    MaxOffset = Offset;
                }
            }
        } else if (BlockPtr->Type == xJavaBlock::TYPE_SWITCH_DECLARATION) {
            xJavaBlock * LastBlockPtr = nullptr;
            xJavaBlock * PreviousBlockPtr = nullptr;

            for (auto & SwitchCase : BlockPtr->SwitchCases) {
                xJavaBlock * SwitchCaseBlockPtr = SwitchCase.BlockPtr;
                if (!LastBlockPtr || (LastBlockPtr->FromOffset < SwitchCaseBlockPtr->FromOffset)) {
                    PreviousBlockPtr = LastBlockPtr;
                    LastBlockPtr = SwitchCaseBlockPtr;
                }
            }
            if (PreviousBlockPtr) {
                Offset = CheckSynchronizedBlockOffset(PreviousBlockPtr);
                if (MaxOffset < Offset) {
                    MaxOffset = Offset;
                }
            }
        }

        return MaxOffset;
    }

    xJavaBlock * xJavaControlFlowGraph::SearchEndBasicBlock(const xBitSet & MemberIndexes, size_t MaxOffset, const xJavaBlockPtrSet & Members)
    {
        xJavaBlock * EndBlockPtr = &xJavaBlock::End;
        for (auto & Member : Members) {
            switch (Member->Type) {
                case xJavaBlock::TYPE_CONDITIONAL_BRANCH:
                {
                    auto BlockPtr = Member->BranchBlockPtr;
                    if (!MemberIndexes[BlockPtr->Index] && (MaxOffset < BlockPtr->FromOffset)) {
                        EndBlockPtr = BlockPtr;
                        MaxOffset = BlockPtr->FromOffset;
                        break;
                    }
                }
                case xJavaBlock::TYPE_STATEMENTS:
                case xJavaBlock::TYPE_GOTO:
                {
                    auto BlockPtr = Member->NextBlockPtr;
                    if (!MemberIndexes[BlockPtr->Index] && (MaxOffset < BlockPtr->FromOffset)) {
                        EndBlockPtr = BlockPtr;
                        MaxOffset = BlockPtr->FromOffset;
                    }
                    break;
                }
                case xJavaBlock::TYPE_SWITCH_DECLARATION:
                {
                    for (auto & SwitchCase : Member->SwitchCases) {
                        auto BlockPtr = SwitchCase.BlockPtr;
                        if (!MemberIndexes[BlockPtr->Index] && (MaxOffset < BlockPtr->FromOffset)) {
                            EndBlockPtr = BlockPtr;
                            MaxOffset = BlockPtr->FromOffset;
                        }
                    }
                    break;
                }
                case xJavaBlock::TYPE_TRY_DECLARATION:
                {
                    auto BlockPtr = Member->NextBlockPtr;
                    if (!MemberIndexes[BlockPtr->Index] && (MaxOffset < BlockPtr->FromOffset)) {
                        EndBlockPtr = BlockPtr;
                        MaxOffset = BlockPtr->FromOffset;
                    }
                    for (auto & ExceptionHandler : Member->ExceptionHandlers) {
                        BlockPtr = ExceptionHandler.BlockPtr;
                        if (!MemberIndexes[BlockPtr->Index] && (MaxOffset < BlockPtr->FromOffset)) {
                            EndBlockPtr = BlockPtr;
                            MaxOffset = BlockPtr->FromOffset;
                        }
                    }
                    break;
                }
                default: break;
            }
        }
        return EndBlockPtr;
    }

    xJavaLoop xJavaControlFlowGraph::MakeLoop(xJavaBlock * StartBlockPtr, xBitSet & SearchZoneIndexes, xBitSet & MemberIndexes)
    {
        size_t Length = BlockPtrList.size();
        assert(Length);

        size_t MaxOffset = 0;
        for (size_t i = 0; i < Length; ++i) {
            if (!MemberIndexes[i]) {
                continue;
            }
            auto Offset = CheckMaxOffset(BlockPtrList[i]);
            if (MaxOffset < Offset) {
                MaxOffset = Offset;
            }
        }

        // // Extend members
        MemberIndexes.clear();
        RecursiveForwardSearchLoopMemberIndexes(MemberIndexes, SearchZoneIndexes, StartBlockPtr, MaxOffset);

        auto Members = xJavaBlockPtrSet();
        for (size_t i = 0; i < Length; ++i) {
            if (MemberIndexes[i]) {
                Members.insert(BlockPtrList[i]);
            }
        }

        // // Search 'end' block
        xJavaBlock * EndBlockPtr = &xJavaBlock::End;

        if (StartBlockPtr->Type == xJavaBlock::TYPE_CONDITIONAL_BRANCH) {
            // First, check natural 'end' blocks
            auto Index = StartBlockPtr->BranchBlockPtr->Index;
            if (!MemberIndexes[Index]) {
                EndBlockPtr = EndBlockPtr->BranchBlockPtr;
            } else {
                Index = StartBlockPtr->NextBlockPtr->Index;
                if (!MemberIndexes[Index]) {
                    EndBlockPtr = StartBlockPtr->NextBlockPtr;
                }
            }
        }

        if (EndBlockPtr->IsTheEnd()) {
            // Not found, check all member blocks
            EndBlockPtr = SearchEndBasicBlock(MemberIndexes, MaxOffset, Members);

            if (!(EndBlockPtr->Type & (xJavaBlock::TYPE_END | xJavaBlock::TYPE_RETURN | xJavaBlock::TYPE_LOOP_START | xJavaBlock::TYPE_LOOP_CONTINUE | xJavaBlock::TYPE_LOOP_END)) &&
                (EndBlockPtr->Predecessors.size() == 1)) {
                // && (EndBlockPtr->Predecessors[0].getLastLineNumber() + 1 >= end.getFirstLineNumber())) // TODO: check if it works w/o line number info
                auto Set = xJavaBlockPtrSet();

                if (RecursiveForwardSearchLastLoopMemberIndexes(Members, SearchZoneIndexes, Set, EndBlockPtr, nullptr)) {

                    for (auto & MemberBlockPtr : Set) {
                        Members.insert(MemberBlockPtr);
                    }

                    for (auto & MemberBlockPtr : Set) {
                        if (!MemberBlockPtr->IsTheEnd()) {
                            MemberIndexes[MemberBlockPtr->Index];
                        }
                    }

                    EndBlockPtr = SearchEndBasicBlock(MemberIndexes, MaxOffset, Set);
                }
            }
        }

        // // Extend last member
        if (EndBlockPtr != &xJavaBlock::End) {
            auto M = Members; // copy
            auto Set = xJavaBlockPtrSet();

            for (auto & Member : M) {
                if ((Member->Type == xJavaBlock::TYPE_CONDITIONAL_BRANCH) && (Member != StartBlockPtr)) {
                    Set.clear();
                    if (RecursiveForwardSearchLastLoopMemberIndexes(Members, SearchZoneIndexes, Set, Member->NextBlockPtr, EndBlockPtr)) {
                        for(auto BlockPtr : Set) {
                            Members.insert(BlockPtr);
                        }
                    }
                    Set.clear();
                    if (RecursiveForwardSearchLastLoopMemberIndexes(Members, SearchZoneIndexes, Set, Member->BranchBlockPtr, EndBlockPtr)) {
                        for(auto BlockPtr : Set) {
                            Members.insert(BlockPtr);
                        }
                    }
                }
            }
        }

        return xJavaLoop{ StartBlockPtr, EndBlockPtr, Members };
    }

    std::vector<xJavaLoop> xJavaControlFlowGraph::IdentifyNaturalLoops(const std::vector<xBitSet> & ArrayOfDominatorIndexes)
    {
        auto Length = BlockPtrList.size();
        auto ArrayOfMemberIndexes = std::vector<xBitSet>(Length);

        // Identify loop members
        for (size_t i = 0 ; i < Length ; ++i) {
            auto BlockPtr = BlockPtrList[i];
            auto & DominatorIndexes = ArrayOfDominatorIndexes[i];

            switch (BlockPtr->Type) {
                case xJavaBlock::TYPE_CONDITIONAL_BRANCH: {
                    auto Index = BlockPtr->BranchBlockPtr->Index;
                        // X_DEBUG_PRINTF("-->B %zi\n", Index);
                    if (!BlockPtr->BranchBlockPtr->IsTheEnd() && DominatorIndexes[Index]) {
                        // 'branch' is a dominator -> Back edge found
                        ArrayOfMemberIndexes[Index] = SearchLoopMemberIndexes(Length, ArrayOfMemberIndexes[Index], BlockPtr, BlockPtr->BranchBlockPtr);
                    }
                    // pass through
                }
                case xJavaBlock::TYPE_STATEMENTS:
                case xJavaBlock::TYPE_GOTO: {
                    auto Index = BlockPtr->NextBlockPtr->Index;
                        // X_DEBUG_PRINTF("-->N %zi\n", Index);
                    if (!BlockPtr->NextBlockPtr->IsTheEnd() && DominatorIndexes[Index]) {
                        // 'next' is a dominator -> Back edge found
                        ArrayOfMemberIndexes[Index] = SearchLoopMemberIndexes(Length, ArrayOfMemberIndexes[Index], BlockPtr, BlockPtr->NextBlockPtr);
                    }
                    break;
                }
                case xJavaBlock::TYPE_SWITCH_DECLARATION: {
                    for (auto SwitchCase : BlockPtr->SwitchCases) {
                        auto Index = SwitchCase.BlockPtr->Index;
                        // X_DEBUG_PRINTF("-->S %zi\n", Index);
                        if (!SwitchCase.BlockPtr->IsTheEnd() && DominatorIndexes[Index]) {
                            // 'switchCase' is a dominator -> Back edge found
                            ArrayOfMemberIndexes[Index] = SearchLoopMemberIndexes(Length, ArrayOfMemberIndexes[Index], BlockPtr, SwitchCase.BlockPtr);
                        }
                    }
                    break;
                }
                default: break;
            }
        }

        // for(size_t i = 0 ; i < ArrayOfMemberIndexes.size(); ++i) {
        //     X_DEBUG_PRINTF("%zi %s\n", i, ToString(ArrayOfMemberIndexes[i]).c_str());
        // }

        // Loops & 'try' statements
        for (size_t i = 0 ; i < Length; ++i) {
            auto & MemberIndexes = ArrayOfMemberIndexes[i];
            if (MemberIndexes.empty()) {
                continue;
            }
            size_t MaxOffset = 0;
            for (size_t j = 0; j < Length; ++j) {
                if (MemberIndexes[j]) {
                    size_t Offset = BlockPtrList[j]->FromOffset;
                    if (MaxOffset < Offset) {
                        MaxOffset = Offset;
                    }
                }
            }

            auto StartBlockPtr = BlockPtrList[i];
            auto & StartDominatorIndexes = ArrayOfDominatorIndexes[i];

            if ((StartBlockPtr->Type == xJavaBlock::TYPE_TRY_DECLARATION)
                && (MaxOffset != StartBlockPtr->FromOffset)
                && (MaxOffset < StartBlockPtr->ExceptionHandlers[0].BlockPtr->FromOffset)) {

                // 'try' statement outside the loop
                auto NewStartBlockPtr = StartBlockPtr->NextBlockPtr;
                auto & NewStartBlockPredecessors = NewStartBlockPtr->Predecessors;

                // Loop in 'try' statement
                for (auto Iter = NewStartBlockPredecessors.begin(); Iter != NewStartBlockPredecessors.end();) {
                    auto PredecessorBlockPtr = *Iter;
                    if (!StartDominatorIndexes[PredecessorBlockPtr->Index]) {
                        Iter = NewStartBlockPredecessors.erase(Iter);
                        PredecessorBlockPtr->Replace(StartBlockPtr, NewStartBlockPtr);
                        NewStartBlockPtr->Predecessors.insert(PredecessorBlockPtr);
                    }
                    else {
                        ++Iter;
                    }
                }

                MemberIndexes[StartBlockPtr->Index] = false;
                ArrayOfMemberIndexes[NewStartBlockPtr->Index] = std::move(MemberIndexes);
            }
        }

        X_DEBUG_PRINTF("xxxxxxxxxxxxxxxxx\n");
        for(size_t i = 0 ; i < ArrayOfMemberIndexes.size(); ++i) {
            X_DEBUG_PRINTF("%zi %s\n", i, ToString(ArrayOfMemberIndexes[i]).c_str());
        }

        // Build loops
        auto Loops = std::vector<xJavaLoop>(); // return value
        for (size_t i = 0; i < Length; ++i) {
            if (ArrayOfMemberIndexes[i].empty()) {
                continue;
            }
            auto & MemberIndexes = ArrayOfMemberIndexes[i];

            // Unoptimize loop
            auto StartBlockPtr = BlockPtrList[i];
            auto & StartDominatorIndexes = ArrayOfDominatorIndexes[i];

            auto SearchZoneIndexes = StartDominatorIndexes; // copy
            for (auto BitReference : SearchZoneIndexes) { // flip
                BitReference = !BitReference;
            }
            auto CheckSearchZoneIndexes = StartDominatorIndexes;
            CheckSearchZoneIndexes.flip();
            assert(SearchZoneIndexes == CheckSearchZoneIndexes);
            SearchZoneIndexes[StartBlockPtr->Index] = true;

            if (StartBlockPtr->Type == xJavaBlock::TYPE_CONDITIONAL_BRANCH) {
                if (StartBlockPtr->NextBlockPtr != StartBlockPtr
                    && StartBlockPtr->BranchBlockPtr != StartBlockPtr
                    && MemberIndexes[StartBlockPtr->NextBlockPtr->Index]
                    && MemberIndexes[StartBlockPtr->BranchBlockPtr->Index]) {

                    auto NextIndexes = xBitSet(Length);
                    auto BranchIndexes = xBitSet(Length);
                    RecursiveForwardSearchLoopMemberIndexes(NextIndexes, MemberIndexes, StartBlockPtr->NextBlockPtr, StartBlockPtr);
                    RecursiveForwardSearchLoopMemberIndexes(BranchIndexes, MemberIndexes, StartBlockPtr->BranchBlockPtr, StartBlockPtr);

                    auto CommonMemberIndexes = NextIndexes; // copy
                    for (size_t j = 0; j < CommonMemberIndexes.size(); ++j) {
                        CommonMemberIndexes[j] = CommonMemberIndexes[j] & BranchIndexes[j];
                    }
                    auto OnlyLoopHeaderIndexes = xBitSet(Length);
                    OnlyLoopHeaderIndexes[i] = true;

                    if (CommonMemberIndexes == OnlyLoopHeaderIndexes) {
                        // Only 'start' is the common basic block -> Split loop
                        Loops.push_back(MakeLoop(StartBlockPtr, SearchZoneIndexes, MemberIndexes));

                        BranchIndexes.flip();
                        for (size_t j = 0; j < SearchZoneIndexes.size(); ++j) {
                            SearchZoneIndexes[j] = SearchZoneIndexes[j] & BranchIndexes[j];
                        }
                        SearchZoneIndexes[StartBlockPtr->Index];
                        Loops.push_back(MakeLoop(StartBlockPtr, SearchZoneIndexes, NextIndexes));
                    }
                    else {
                        Loops.push_back(MakeLoop(StartBlockPtr, SearchZoneIndexes, MemberIndexes));
                    }
                }
                else {
                    Loops.push_back(MakeLoop(StartBlockPtr, SearchZoneIndexes, MemberIndexes));
                }
            }
            else {
                Loops.push_back(MakeLoop(StartBlockPtr, SearchZoneIndexes, MemberIndexes));
            }
        }

        std::sort(Loops.begin(), Loops.end(), xJavaLoop::xComparator());
        return Loops;
    }

    static xJavaBlock * RecheckEndBlock(xJavaBlockPtrSet & Members, xJavaBlockPtr EndBlockPtr)
    {
        do {
    //         boolean flag = false;

    //         for (BasicBlock predecessor : end.getPredecessors()) {
    //             if (!members.contains(predecessor)) {
    //                 flag = true;
    //                 break;
    //             }
    //         }

    //         if (flag) {
    //             break;
    //         }

    //         // Search new 'end' block
    //         BasicBlock newEnd = null;
            auto NewEndBlockPtr = xJavaBlockPtr(nullptr);

    //         for (BasicBlock member : members) {
    //             if (member.matchType(GROUP_SINGLE_SUCCESSOR)) {
    //                 BasicBlock bb = member.getNext();
    //                 if ((bb != end) && !members.contains(bb)) {
    //                     newEnd = bb;
    //                     break;
    //                 }
    //             } else if (member->Type == TYPE_CONDITIONAL_BRANCH) {
    //                 BasicBlock bb = member.getNext();
    //                 if ((bb != end) && !members.contains(bb)) {
    //                     newEnd = bb;
    //                     break;
    //                 }
    //                 bb = member->BranchBlockPtr;
    //                 if ((bb != end) && !members.contains(bb)) {
    //                     newEnd = bb;
    //                     break;
    //                 }
    //             }
    //         }

    //         if ((newEnd == null) || (end.getFromOffset() >= newEnd.getFromOffset())) {
    //             break;
    //         }

            // Replace 'end' block
            if (EndBlockPtr->Type & (xJavaBlock::TYPE_RETURN | xJavaBlock::TYPE_RETURN_VALUE | xJavaBlock::TYPE_THROW)) {
                Members.insert(EndBlockPtr);
                EndBlockPtr = NewEndBlockPtr;
            } else if ((EndBlockPtr->Type & (xJavaBlock::GROUP_SINGLE_SUCCESSOR)) && (EndBlockPtr->NextBlockPtr == NewEndBlockPtr)) {
                Members.insert(EndBlockPtr);
                EndBlockPtr = NewEndBlockPtr;
            } else {
                break;
            }

        } while (false);

        return EndBlockPtr;
    }

    xJavaBlock * xJavaControlFlowGraph::ReduceLoop(xJavaLoop & Loop)
    {
        auto StartBlockPtr = Loop.StartBlockPtr;
        auto EndBlockPtr = Loop.EndBlockPtr;
        auto & Members = Loop.MemberBlocks;

        assert(StartBlockPtr->GetControlFlowGraph() == this);
        assert(EndBlockPtr->GetControlFlowGraph() == this);

        size_t ToOffset = StartBlockPtr->ToOffset;

        // Recheck 'end' block
        EndBlockPtr = RecheckEndBlock(Members, EndBlockPtr);

        // Build new basic block for Loop
        auto LoopBlockPtr = NewBlock(xJavaBlock::TYPE_LOOP, StartBlockPtr->FromOffset, StartBlockPtr->ToOffset);

        // Update predecessors
        do {
            auto & StartPredecessors = StartBlockPtr->Predecessors;
            for (auto Iter = StartPredecessors.begin(); Iter != StartPredecessors.end(); ) {
                auto & PredecessorBlockPtr = *Iter;
                if (Members.find(PredecessorBlockPtr) == Members.end()) {
                    PredecessorBlockPtr->Replace(StartBlockPtr, LoopBlockPtr);
                    LoopBlockPtr->Predecessors.insert(PredecessorBlockPtr);
                    Iter = StartPredecessors.erase(Iter);
                } else {
                    ++Iter;
                }
            }
            LoopBlockPtr->FirstSubBlockPtr = StartBlockPtr;
        } while(false);

        // Set &xJavaBlock::LoopStart, &xJavaBlock::LoopEnd and TYPE_JUMP
        for (auto MemberBlockPtr : Members) {
            if (MemberBlockPtr->Type & (xJavaBlock::GROUP_SINGLE_SUCCESSOR)) {
                auto BlockPtr = MemberBlockPtr->NextBlockPtr;

                if (BlockPtr == StartBlockPtr) {
                    MemberBlockPtr->NextBlockPtr = &xJavaBlock::LoopStart;
                } else if (BlockPtr == EndBlockPtr) {
                    MemberBlockPtr->NextBlockPtr = &xJavaBlock::LoopEnd;
                } else if ((Members.find(BlockPtr) == Members.end()) && (BlockPtr->Predecessors.size() > 1)) {
                    MemberBlockPtr->NextBlockPtr = NewJumpBlock(MemberBlockPtr, BlockPtr);
                }
            }
            else if (MemberBlockPtr->Type == xJavaBlock::TYPE_CONDITIONAL_BRANCH) {
                auto BlockPtr = MemberBlockPtr->NextBlockPtr;
                if (BlockPtr == StartBlockPtr) {
                    MemberBlockPtr->NextBlockPtr = &xJavaBlock::LoopStart;
                } else if (BlockPtr == EndBlockPtr) {
                    MemberBlockPtr->NextBlockPtr = &xJavaBlock::LoopEnd;
                } else if ((Members.find(BlockPtr) == Members.end()) && (BlockPtr->Predecessors.size() > 1)) {
                    MemberBlockPtr->NextBlockPtr = NewJumpBlock(MemberBlockPtr, BlockPtr);
                }

                BlockPtr = MemberBlockPtr->BranchBlockPtr;
                if (BlockPtr == StartBlockPtr) {
                    MemberBlockPtr->BranchBlockPtr = &xJavaBlock::LoopStart;
                } else if (BlockPtr == EndBlockPtr) {
                    MemberBlockPtr->BranchBlockPtr = &xJavaBlock::LoopEnd;
                } else if ((Members.find(BlockPtr) == Members.end()) && (BlockPtr->Predecessors.size() > 1)) {
                    MemberBlockPtr->BranchBlockPtr = NewJumpBlock(MemberBlockPtr, BlockPtr);
                }
            }
            else if (MemberBlockPtr->Type == xJavaBlock::TYPE_SWITCH_DECLARATION) {
                for (auto & SwitchCase : MemberBlockPtr->SwitchCases) {
                    auto BlockPtr = SwitchCase.BlockPtr;
                    if (BlockPtr == StartBlockPtr) {
                        SwitchCase.BlockPtr = &xJavaBlock::LoopStart;
                    } else if (BlockPtr == EndBlockPtr) {
                        SwitchCase.BlockPtr = &xJavaBlock::LoopEnd;
                    } else if ((Members.find(BlockPtr) == Members.end()) && (BlockPtr->Predecessors.size() > 1)) {
                        SwitchCase.BlockPtr = NewJumpBlock(MemberBlockPtr, BlockPtr);
                    }
                }
            }

            if (ToOffset < MemberBlockPtr->ToOffset) {
                ToOffset = MemberBlockPtr->ToOffset;
            }
        }

        if (EndBlockPtr) {
            LoopBlockPtr->NextBlockPtr = EndBlockPtr;
            EndBlockPtr->Replace(Members, LoopBlockPtr);
        }

        StartBlockPtr->Predecessors.clear();
        LoopBlockPtr->ToOffset = ToOffset;

        return LoopBlockPtr;
    }

    void xJavaControlFlowGraph::ReduceLoop()
    {
        auto ArrayOfDominatorIndexes = BuildDominatorIndexes(BlockPtrList);
        auto Loops = IdentifyNaturalLoops(ArrayOfDominatorIndexes);

        for (size_t i = 0, LoopsLength = Loops.size(); i < LoopsLength; ++i) {
            auto & Loop = Loops[i];
            auto StartBlockPtr = Loop.StartBlockPtr;
            auto LoopBlockPtr = ReduceLoop(Loop);

            // Update other Loops
            for (size_t j = LoopsLength - 1; j > i; --j) {
                auto & OtherLoop = Loops[j];

                if (OtherLoop.StartBlockPtr == StartBlockPtr) {
                    OtherLoop.StartBlockPtr = LoopBlockPtr;
                }

                auto & OtherMembers = OtherLoop.MemberBlocks;
                auto Iter = OtherMembers.find(StartBlockPtr);
                if (Iter != OtherMembers.end()) {
                    for (auto & MemberBlockPtr : Loop.MemberBlocks) {
                        auto MatchIter = OtherMembers.find(MemberBlockPtr);
                        if (MatchIter != OtherMembers.end()) {
                            OtherMembers.erase(MatchIter);
                        }
                    }
                    OtherMembers.insert(LoopBlockPtr);
                }

                if (OtherLoop.EndBlockPtr == StartBlockPtr) {
                    OtherLoop.EndBlockPtr = LoopBlockPtr;
                }
            }
        }
        return;
    }

}