#include <jdc/decompiler/JD_JavaControlFlowGraph.hpp>
#include <jdc/decompiler/JD_JavaControlFlowGraph_Loop.hpp>
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

    static std::vector<xJavaLoop> IdentifyNaturalLoops(std::vector<xJavaBlock*> & BlockPtrList, const std::vector<xBitSet> & ArrayOfDominatorIndexes)
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
                        X_DEBUG_PRINTF("-->B %zi\n", Index);
                    if (DominatorIndexes[Index]) {
                        // 'branch' is a dominator -> Back edge found
                        ArrayOfMemberIndexes[Index] = SearchLoopMemberIndexes(Length, ArrayOfMemberIndexes[Index], BlockPtr, BlockPtr->BranchBlockPtr);
                    }
                    // pass through
                }
                case xJavaBlock::TYPE_STATEMENTS:
                case xJavaBlock::TYPE_GOTO: {
                    auto Index = BlockPtr->NextBlockPtr->Index;
                        X_DEBUG_PRINTF("-->N %zi\n", Index);
                    if (DominatorIndexes[Index]) {
                        // 'next' is a dominator -> Back edge found
                        ArrayOfMemberIndexes[Index] = SearchLoopMemberIndexes(Length, ArrayOfMemberIndexes[Index], BlockPtr, BlockPtr->NextBlockPtr);
                    }
                    break;
                }
                case xJavaBlock::TYPE_SWITCH_DECLARATION: {
                    for (auto SwitchCase : BlockPtr->SwitchCases) {
                        auto Index = SwitchCase.BlockPtr->Index;
                        X_DEBUG_PRINTF("-->S %zi\n", Index);
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

        // Loops & 'try' statements
        for(size_t i = 0 ; i < ArrayOfMemberIndexes.size(); ++i) {
            X_DEBUG_PRINTF("%zi %s\n", i, ToString(ArrayOfMemberIndexes[i]).c_str());
        }

        // Build loops
        auto Loops = std::vector<xJavaLoop>(); // return value

        return Loops;
    }

    void xJavaControlFlowGraph::ReduceLoop()
    {
        auto ArrayOfDominatorIndexes = BuildDominatorIndexes(BlockPtrList);
        auto Loops = IdentifyNaturalLoops(BlockPtrList, ArrayOfDominatorIndexes);

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