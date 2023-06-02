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

    static std::vector<xBitSet> BuildDominatorIndexes(std::vector<xJavaBlock*> & BlockPtrList)
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

    [[maybe_unused]]
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
        // TODO
        Todo();
        return 0;
    }

    static size_t CheckSynchronizedBlockOffset(xJavaBlock * BlockPtr)
    {
        // TODO
        Todo();
        return 0;
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

    xJavaLoop xJavaControlFlowGraph::MakeLoop(xJavaBlock * StartBlockPtr, const xBitSet & SearchZoneIndexes, xBitSet & MemberIndexes)
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

        auto Members = std::set<xJavaBlock*>();
        for (size_t i = 0; i < Length; ++i) {
            if (MemberIndexes[i]) {
                Members.insert(BlockPtrList[i]);
            }
        }

        // // Search 'end' block
        xJavaBlock * EndBlockPtr = this->EndBlockPtr;

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

        if (EndBlockPtr == this->EndBlockPtr) {
            // Not found, check all member blocks
            // EndBlockPtr = SearchEndBasicBlock(MemberIndexes, MaxOffset, Members);

            // if (!(EndBlockPtr->Type & (xJavaBlock::TYPE_END | xJavaBlock::TYPE_RETURN | xJavaBlock::TYPE_LOOP_START | xJavaBlock::TYPE_LOOP_CONTINUE | xJavaBlock::TYPE_LOOP_END)) &&
            //     (EndBlockPtr->Predecessors.size() == 1)) { // TODO: check if it works w/o line number info
            //     // && (EndBlockPtr->Predecessors[0].getLastLineNumber() + 1 >= end.getFirstLineNumber()))
            // {
            //     auto Set = new HashSet<>();

            //     if (RecursiveForwardSearchLastLoopMemberIndexes(members, searchZoneIndexes, set, end, null)) {
            //         members.addAll(set);

            //         for (BasicBlock member : set) {
            //             if (member.getIndex() >= 0) {
            //                 MemberIndexes.set(member.getIndex());
            //             }
            //         }

            //         end = searchEndBasicBlock(MemberIndexes, maxOffset, set);
            //     }
            // }
        }

        // // Extend last member
        // if (end != END) {
        //     HashSet<BasicBlock> m = new HashSet<>(members);
        //     HashSet<BasicBlock> set = new HashSet<>();

        //     for (BasicBlock member : m) {
        //         if ((member.getType() == TYPE_CONDITIONAL_BRANCH) && (member != start)) {
        //             set.clear();
        //             if (recursiveForwardSearchLastLoopMemberIndexes(members, searchZoneIndexes, set, member.getNext(), end)) {
        //                 members.addAll(set);
        //             }
        //             set.clear();
        //             if (recursiveForwardSearchLastLoopMemberIndexes(members, searchZoneIndexes, set, member.getBranch(), end)) {
        //                 members.addAll(set);
        //             }
        //         }
        //     }
        // }

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
                    if (DominatorIndexes[Index]) {
                        // 'branch' is a dominator -> Back edge found
                        ArrayOfMemberIndexes[Index] = SearchLoopMemberIndexes(Length, ArrayOfMemberIndexes[Index], BlockPtr, BlockPtr->BranchBlockPtr);
                    }
                    // pass through
                }
                case xJavaBlock::TYPE_STATEMENTS:
                case xJavaBlock::TYPE_GOTO: {
                    auto Index = BlockPtr->NextBlockPtr->Index;
                        // X_DEBUG_PRINTF("-->N %zi\n", Index);
                    if (DominatorIndexes[Index]) {
                        // 'next' is a dominator -> Back edge found
                        ArrayOfMemberIndexes[Index] = SearchLoopMemberIndexes(Length, ArrayOfMemberIndexes[Index], BlockPtr, BlockPtr->NextBlockPtr);
                    }
                    break;
                }
                case xJavaBlock::TYPE_SWITCH_DECLARATION: {
                    for (auto SwitchCase : BlockPtr->SwitchCases) {
                        auto Index = SwitchCase.BlockPtr->Index;
                        // X_DEBUG_PRINTF("-->S %zi\n", Index);
                        if (DominatorIndexes[Index]) {
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

    void xJavaControlFlowGraph::ReduceLoop()
    {
        auto ArrayOfDominatorIndexes = BuildDominatorIndexes(BlockPtrList);
        auto Loops = IdentifyNaturalLoops(ArrayOfDominatorIndexes);

        // for (int i=0, loopsLength=loops.size(); i<loopsLength; i++) {
        //     Loop loop = loops.get(i);
        //     BasicBlock startBB = loop.getStart();
        //     BasicBlock loopBB = reduceLoop(loop);

        //     // Update other loops
        //     for (int j=loopsLength-1; j>i; j--) {
        //         Loop otherLoop = loops.get(j);

        //         if (otherLoop.getStart() == startBB) {
        //             otherLoop.setStart(loopBB);
        //         }

        //         if (otherLoop.getMembers().contains(startBB)) {
        //             otherLoop.getMembers().removeAll(loop.getMembers());
        //             otherLoop.getMembers().add(loopBB);
        //         }

        //         if (otherLoop.getEnd() == startBB) {
        //             otherLoop.setEnd(loopBB);
        //         }
        //     }
        // }
        return;
    }

}