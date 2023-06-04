#include <jdc/decompiler/JD_JavaControlFlowGraph.hpp>
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

    static bool ReduceConditionalBranch(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JstTargets);
    static bool ReduceSwitchDeclaration(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JstTargets);
    static bool ReduceTryDeclaration(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JstTargets);
    static bool ReduceJsr(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JstTargets);
    static xJavaBlock * SearchUpdateBlockAndCreateContinueLoop(xJavaBlock * BlockPtr, xBitSet & Visited);
    static xJavaBlock * GetLastConditionalBranch(xJavaBlock * BlockPtr, xBitSet & Visited);
    static void RemoveLastContinueLoop(xJavaBlock * BlockPtr);

    bool ReduceConditionalBranch(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JstTargets)
    {
        // TODO
        return true;
    }

    bool ReduceSwitchDeclaration(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JstTargets)
    {
        // TODO
        return true;
    }

    bool ReduceTryDeclaration(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JstTargets)
    {
        // TODO
        return true;
    }

    bool ReduceJsr(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JstTargets)
    {
        // TODO
        return true;
    }

    xJavaBlock * SearchUpdateBlockAndCreateContinueLoop(xJavaBlock * BlockPtr, xBitSet & Visited)
    {
        // TODO
        return nullptr;
    }

    xJavaBlock * GetLastConditionalBranch(xJavaBlock * BlockPtr, xBitSet & Visited)
    {
        if (Visited[BlockPtr->Index]) {
            return nullptr;
        }
        if (BlockPtr->Type & xJavaBlock::GROUP_END) {
            return nullptr;
        }
        Visited[BlockPtr->Index] = true;
        switch (BlockPtr->Type) {
            case xJavaBlock::TYPE_START:
            case xJavaBlock::TYPE_STATEMENTS:
            case xJavaBlock::TYPE_SWITCH_DECLARATION:
            case xJavaBlock::TYPE_TRY_DECLARATION:
            case xJavaBlock::TYPE_JSR:
            case xJavaBlock::TYPE_LOOP:
            case xJavaBlock::TYPE_IF_ELSE:
            case xJavaBlock::TYPE_SWITCH:
            case xJavaBlock::TYPE_TRY:
            case xJavaBlock::TYPE_TRY_JSR:
            case xJavaBlock::TYPE_TRY_ECLIPSE:
                return GetLastConditionalBranch(BlockPtr->NextBlockPtr, Visited);
            case xJavaBlock::TYPE_IF:
            case xJavaBlock::TYPE_CONDITIONAL_BRANCH:
            case xJavaBlock::TYPE_CONDITION:
            case xJavaBlock::TYPE_CONDITION_OR:
            case xJavaBlock::TYPE_CONDITION_AND:
            {
                auto LastConditionalBranchPtr = GetLastConditionalBranch(BlockPtr->BranchBlockPtr, Visited);
                if (LastConditionalBranchPtr) {
                    return LastConditionalBranchPtr;
                }
                LastConditionalBranchPtr = GetLastConditionalBranch(BlockPtr->NextBlockPtr, Visited);
                if (LastConditionalBranchPtr) {
                    return LastConditionalBranchPtr;
                }
                return BlockPtr;
            }
            default:
                break;
        }
        Fatal("Unprocessed branch");
        return nullptr;
    }

    void RemoveLastContinueLoop(xJavaBlock * BlockPtr)
    {
        auto Visited = xBitSet();
        auto NextBlockPtr = BlockPtr->NextBlockPtr;

        while (!(NextBlockPtr->Type & xJavaBlock::GROUP_END) && !Visited[NextBlockPtr->Index]) {
            Visited[NextBlockPtr->Index] = true;
            BlockPtr = NextBlockPtr;
            NextBlockPtr = BlockPtr->NextBlockPtr;
        }

        if (NextBlockPtr == &xJavaBlock::LoopContinue) {
            BlockPtr->NextBlockPtr = &xJavaBlock::End;
        }
    }

    void xJavaControlFlowGraph::ChangeEndLoopToJump(xBitSet & Visited, xJavaBlock * TargetPtr, xJavaBlock * BlockPtr)
    {
        if (!(BlockPtr->Type & xJavaBlock::GROUP_END) && !Visited[BlockPtr->Index]) {
            Visited[BlockPtr->Index] = true;

            switch (BlockPtr->Type) {
                case xJavaBlock::TYPE_CONDITIONAL_BRANCH:
                case xJavaBlock::TYPE_JSR:
                case xJavaBlock::TYPE_CONDITION:
                {
                    if (BlockPtr->BranchBlockPtr == &xJavaBlock::LoopEnd) {
                        BlockPtr->BranchBlockPtr = NewJumpBlock(BlockPtr, TargetPtr);
                    } else {
                        ChangeEndLoopToJump(Visited, TargetPtr, BlockPtr->BranchBlockPtr);
                    }
                    // pass through
                }
                case xJavaBlock::TYPE_START:
                case xJavaBlock::TYPE_STATEMENTS:
                case xJavaBlock::TYPE_GOTO:
                case xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR:
                case xJavaBlock::TYPE_LOOP:
                {
                    if (BlockPtr->NextBlockPtr == &xJavaBlock::LoopEnd) {
                        BlockPtr->NextBlockPtr = NewJumpBlock(BlockPtr, TargetPtr);
                    } else {
                        ChangeEndLoopToJump(Visited, TargetPtr, BlockPtr->NextBlockPtr);
                    }
                    break;
                }
                case xJavaBlock::TYPE_TRY:
                case xJavaBlock::TYPE_TRY_JSR:
                case xJavaBlock::TYPE_TRY_ECLIPSE:
                {
                    if (BlockPtr->FirstSubBlockPtr == &xJavaBlock::LoopEnd) {
                        BlockPtr->FirstSubBlockPtr = NewJumpBlock(BlockPtr, TargetPtr);
                    } else {
                        ChangeEndLoopToJump(Visited, TargetPtr, BlockPtr->FirstSubBlockPtr);
                    }
                    // pass through
                }
                case xJavaBlock::TYPE_TRY_DECLARATION:
                {
                    for (auto & ExceptionHandler : BlockPtr->ExceptionHandlers) {
                        if (ExceptionHandler.BlockPtr == &xJavaBlock::LoopEnd) {
                            ExceptionHandler.BlockPtr = NewJumpBlock(BlockPtr, TargetPtr);
                        } else {
                            ChangeEndLoopToJump(Visited, TargetPtr, ExceptionHandler.BlockPtr);
                        }
                    }
                    break;
                }
                case xJavaBlock::TYPE_IF_ELSE:
                case xJavaBlock::TYPE_TERNARY_OPERATOR:
                {
                    if (BlockPtr->SecondSubBlockPtr == &xJavaBlock::LoopEnd) {
                        BlockPtr->SecondSubBlockPtr = NewJumpBlock(BlockPtr, TargetPtr);
                    } else {
                        ChangeEndLoopToJump(Visited, TargetPtr, BlockPtr->SecondSubBlockPtr);
                    }
                    // pass through
                }
                case xJavaBlock::TYPE_IF:
                {
                    if (BlockPtr->FirstSubBlockPtr == &xJavaBlock::LoopEnd) {
                        BlockPtr->FirstSubBlockPtr = NewJumpBlock(BlockPtr, TargetPtr);
                    } else {
                        ChangeEndLoopToJump(Visited, TargetPtr, BlockPtr->FirstSubBlockPtr);
                    }
                    if (BlockPtr->NextBlockPtr == &xJavaBlock::LoopEnd) {
                        BlockPtr->NextBlockPtr = NewJumpBlock(BlockPtr, TargetPtr);
                    } else {
                        ChangeEndLoopToJump(Visited, TargetPtr, BlockPtr->NextBlockPtr);
                    }
                    break;
                }
                case xJavaBlock::TYPE_CONDITION_OR:
                case xJavaBlock::TYPE_CONDITION_AND:
                {
                    if (BlockPtr->FirstSubBlockPtr == &xJavaBlock::LoopEnd) {
                        BlockPtr->FirstSubBlockPtr = NewJumpBlock(BlockPtr, TargetPtr);
                    } else {
                        ChangeEndLoopToJump(Visited, TargetPtr, BlockPtr->FirstSubBlockPtr);
                    }
                    if (BlockPtr->SecondSubBlockPtr == &xJavaBlock::LoopEnd) {
                        BlockPtr->SecondSubBlockPtr = NewJumpBlock(BlockPtr, TargetPtr);
                    } else {
                        ChangeEndLoopToJump(Visited, TargetPtr, BlockPtr->SecondSubBlockPtr);
                    }
                    break;
                }
                case xJavaBlock::TYPE_SWITCH:
                {
                    if (BlockPtr->NextBlockPtr == &xJavaBlock::LoopEnd) {
                        BlockPtr->NextBlockPtr = NewJumpBlock(BlockPtr, TargetPtr);
                    } else {
                        ChangeEndLoopToJump(Visited, TargetPtr, BlockPtr->NextBlockPtr);
                    }
                    // pass through
                }
                case xJavaBlock::TYPE_SWITCH_DECLARATION:
                {
                    for (auto & SwitchCase : BlockPtr->SwitchCases) {
                        if (SwitchCase.BlockPtr == &xJavaBlock::LoopEnd) {
                            SwitchCase.BlockPtr = NewJumpBlock(BlockPtr, TargetPtr);
                        } else {
                            ChangeEndLoopToJump(Visited, TargetPtr, SwitchCase.BlockPtr);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    bool xJavaControlFlowGraph::ReduceLoop(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JstTargets)
    {
        auto VisitedBackup = Visited;
        auto Reduced = Reduce(BlockPtr->FirstSubBlockPtr, Visited, JstTargets);

        if (!Reduced) {
            Visited = VisitedBackup; // restore
            auto VisitedMembers = xBitSet();
            auto UpdateBlockPtr = SearchUpdateBlockAndCreateContinueLoop(BlockPtr->FirstSubBlockPtr, VisitedMembers);
            Reduced = Reduce(BlockPtr->FirstSubBlockPtr, Visited, JstTargets);

            if (UpdateBlockPtr) {
                RemoveLastContinueLoop(BlockPtr->FirstSubBlockPtr->FirstSubBlockPtr);
                auto IfBasicBlock = NewBlock(xJavaBlock::TYPE_IF, BlockPtr->FirstSubBlockPtr->FromOffset, BlockPtr->ToOffset);

                IfBasicBlock->ConditionBlockPtr = &xJavaBlock::End;
                IfBasicBlock->FirstSubBlockPtr = BlockPtr->FirstSubBlockPtr;
                IfBasicBlock->NextBlockPtr = UpdateBlockPtr;
                UpdateBlockPtr->Predecessors.insert(IfBasicBlock);
                BlockPtr->FirstSubBlockPtr = IfBasicBlock;
            }

            if (!Reduced) {
                VisitedMembers.clear();
                auto ConditionalBranchBlockPtr = GetLastConditionalBranch(BlockPtr->FirstSubBlockPtr, VisitedMembers);
                if (ConditionalBranchBlockPtr && (ConditionalBranchBlockPtr->NextBlockPtr == &xJavaBlock::LoopStart)) {
                    VisitedMembers.clear();
                    VisitedMembers[ConditionalBranchBlockPtr->Index] = true;
                    ChangeEndLoopToJump(VisitedMembers, BlockPtr->NextBlockPtr, BlockPtr->FirstSubBlockPtr);

                    auto NewLoopBlockPtr = CopyBlock(BlockPtr);
                    auto & Predecessors = ConditionalBranchBlockPtr->Predecessors;
                    for (auto & PredecessorBlockPtr : Predecessors) {
                        PredecessorBlockPtr->Replace(ConditionalBranchBlockPtr, &xJavaBlock::LoopEnd);
                    }

                    NewLoopBlockPtr->NextBlockPtr = ConditionalBranchBlockPtr;
                    Predecessors.clear();
                    Predecessors.insert(NewLoopBlockPtr);
                    BlockPtr->FirstSubBlockPtr = NewLoopBlockPtr;

                    VisitedMembers.clear();
                    Reduced = Reduce(NewLoopBlockPtr, VisitedMembers, JstTargets);
                }
            }
        }

        return Reduced & Reduce(BlockPtr->NextBlockPtr, Visited, JstTargets);
    }

    bool xJavaControlFlowGraph::Reduce(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JstTargets)
    {
        if (Visited[BlockPtr->Index]) {
            return true;
        }
        if (BlockPtr->Type & xJavaBlock::GROUP_END) {
            return true;
        }
        switch(BlockPtr->Type) {
            case xJavaBlock::TYPE_START:
            case xJavaBlock::TYPE_STATEMENTS:
            case xJavaBlock::TYPE_IF:
            case xJavaBlock::TYPE_IF_ELSE:
            case xJavaBlock::TYPE_SWITCH:
            case xJavaBlock::TYPE_TRY:
            case xJavaBlock::TYPE_TRY_JSR:
            case xJavaBlock::TYPE_TRY_ECLIPSE:
            case xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR:
                return Reduce(BlockPtr->NextBlockPtr, Visited, JstTargets);
            case xJavaBlock::TYPE_CONDITIONAL_BRANCH:
            case xJavaBlock::TYPE_CONDITION:
            case xJavaBlock::TYPE_CONDITION_OR:
            case xJavaBlock::TYPE_CONDITION_AND:
            case xJavaBlock::TYPE_CONDITION_TERNARY_OPERATOR:
                return ReduceConditionalBranch(BlockPtr, Visited, JstTargets);
            case xJavaBlock::TYPE_SWITCH_DECLARATION:
                return ReduceSwitchDeclaration(BlockPtr, Visited, JstTargets);
            case xJavaBlock::TYPE_TRY_DECLARATION:
                return ReduceTryDeclaration(BlockPtr, Visited, JstTargets);
            case xJavaBlock::TYPE_JSR:
                return ReduceJsr(BlockPtr, Visited, JstTargets);
            case xJavaBlock::TYPE_LOOP:
                return ReduceLoop(BlockPtr, Visited, JstTargets);
            default:
                break;
        };

        Fatal("Unprocessed branch");
        return true;
    }

    /**
     * @brief Final reduce,
     *
     * @return true
     * @return false
     */
    bool xJavaControlFlowGraph::Reduce()
    {
        auto StartBlockPtr = BlockPtrList[0];
        auto Visited = xBitSet();
        auto JstTargets = xBitSet();

        return Reduce(StartBlockPtr, Visited, JstTargets);
    }


}