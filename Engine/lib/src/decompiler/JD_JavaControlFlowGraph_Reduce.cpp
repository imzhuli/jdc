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
    static xJavaBlock * SearchUpdateBlockAndCreateContinueLoop(xBitSet & Visited, xJavaBlock * BlockPtr);
    static xJavaBlock * SearchUpdateBlockAndCreateContinueLoop(xBitSet & Visited, xJavaBlock * BlockPtr, xJavaBlock * SubBlockPtr);
    static xJavaBlock * GetLastConditionalBranch(xJavaBlock * BlockPtr, xBitSet & Visited);
    static void RemoveLastContinueLoop(xJavaBlock * BlockPtr);

    bool ReduceConditionalBranch(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JstTargets)
    {
        // TODO
        Todo();
        return true;
    }

    bool ReduceSwitchDeclaration(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JstTargets)
    {
        // TODO
        Todo();
        return true;
    }

    bool ReduceTryDeclaration(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JstTargets)
    {
        // TODO
        Todo();
        return true;
    }

    bool xJavaControlFlowGraph::ReduceJsr(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JstTargets)
    {
        auto BranchBlockPtr = BlockPtr->BranchBlockPtr;
        auto Reduced = Reduce(BlockPtr->NextBlockPtr, Visited, JstTargets) & Reduce(BranchBlockPtr, Visited, JstTargets);

        if ((BranchBlockPtr->Index >= 0) && JstTargets[BranchBlockPtr->Index]) {
            // Reduce JSR
            auto Delta = BlockPtr->ToOffset - BlockPtr->FromOffset;
            if (Delta > 3) {
                auto OpCode = BlockPtr->GetLastOpCode();

                if (OpCode == 168) { // JSR
                    BlockPtr->Type = xJavaBlock::TYPE_STATEMENTS;
                    BlockPtr->ToOffset = BlockPtr->ToOffset - 3;
                    BranchBlockPtr->Predecessors.erase(BranchBlockPtr->Predecessors.find(BlockPtr));
                    return true;
                } else if (Delta > 5) { // JSR_W
                    BlockPtr->Type = xJavaBlock::TYPE_STATEMENTS;
                    BlockPtr->ToOffset = BlockPtr->ToOffset - 5;
                    BranchBlockPtr->Predecessors.erase(BranchBlockPtr->Predecessors.find(BlockPtr));
                    return true;
                }
            }

            // Delete JSR
            BlockPtr->Type = xJavaBlock::TYPE_DELETED;
            BranchBlockPtr->Predecessors.erase(BranchBlockPtr->Predecessors.find(BlockPtr));
            auto & NextPredecessors = BlockPtr->NextBlockPtr->Predecessors;
            NextPredecessors.erase(NextPredecessors.find(BlockPtr));

            for (auto & PredecessorPtr : BlockPtr->Predecessors) {
                PredecessorPtr->Replace(BlockPtr, BlockPtr->NextBlockPtr);
                NextPredecessors.insert(PredecessorPtr);
            }

            return true;
        }

        if (BlockPtr->BranchBlockPtr->Predecessors.size() > 1) {
            // Aggregate JSR
            auto NextBlockPtr = BlockPtr->NextBlockPtr;
            auto & BranchPredecessors = BlockPtr->BranchBlockPtr->Predecessors;
            // Iterator<BasicBlock> iterator = BlockPtr->BranchBlockPtr->Predecessors.iterator();
            for (auto Iter = BranchPredecessors.begin(); Iter != BranchPredecessors.end();) {
                auto PredecessorBlockPtr = *Iter;
                if ((PredecessorBlockPtr != BlockPtr) &&
                    (PredecessorBlockPtr->Type == xJavaBlock::TYPE_JSR) &&
                    (PredecessorBlockPtr->NextBlockPtr == NextBlockPtr)) {

                    for (auto & PredecessorPredecessorBlockPtr : PredecessorBlockPtr->Predecessors) {
                        PredecessorPredecessorBlockPtr->Replace(PredecessorBlockPtr, BlockPtr);
                        BlockPtr->Predecessors.insert(PredecessorPredecessorBlockPtr);
                    }
                    NextBlockPtr->Predecessors.erase(NextBlockPtr->Predecessors.find(PredecessorBlockPtr));
                    Iter = BranchPredecessors.erase(Iter);
                    Reduced = true;
                }
                else {
                    ++Iter;
                }
            }
        }

        return Reduced;
    }

    xJavaBlock * SearchUpdateBlockAndCreateContinueLoop(xBitSet & Visited, xJavaBlock * BlockPtr)
    {
        auto UpdateBasicBlock = xJavaBlockPtr();

        if (!(BlockPtr->Type & xJavaBlock::GROUP_END) && !Visited[BlockPtr->Index]) {
            Visited[BlockPtr->Index] = true;
            switch (BlockPtr->Type) {
                case xJavaBlock::TYPE_CONDITIONAL_BRANCH:
                case xJavaBlock::TYPE_JSR:
                case xJavaBlock::TYPE_CONDITION:
                case xJavaBlock::TYPE_CONDITION_TERNARY_OPERATOR:
                    UpdateBasicBlock = SearchUpdateBlockAndCreateContinueLoop(Visited, BlockPtr, BlockPtr->BranchBlockPtr);
                    // pass through
                case xJavaBlock::TYPE_START:
                case xJavaBlock::TYPE_STATEMENTS:
                case xJavaBlock::TYPE_GOTO:
                case xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR:
                case xJavaBlock::TYPE_LOOP:
                    if (!UpdateBasicBlock) {
                        UpdateBasicBlock = SearchUpdateBlockAndCreateContinueLoop(Visited, BlockPtr, BlockPtr->NextBlockPtr);
                    }
                    break;
                case xJavaBlock::TYPE_TRY:
                case xJavaBlock::TYPE_TRY_JSR:
                case xJavaBlock::TYPE_TRY_ECLIPSE:
                    UpdateBasicBlock = SearchUpdateBlockAndCreateContinueLoop(Visited, BlockPtr, BlockPtr->FirstSubBlockPtr);
                    // pass through
                case xJavaBlock::TYPE_TRY_DECLARATION:
                    for (auto & ExceptionHandler : BlockPtr->ExceptionHandlers) {
                        if (!UpdateBasicBlock) {
                            UpdateBasicBlock = SearchUpdateBlockAndCreateContinueLoop(Visited, BlockPtr, ExceptionHandler.BlockPtr);
                        }
                    }
                    if (!UpdateBasicBlock) {
                        UpdateBasicBlock = SearchUpdateBlockAndCreateContinueLoop(Visited, BlockPtr, BlockPtr->NextBlockPtr);
                    }
                    break;
                case xJavaBlock::TYPE_IF_ELSE:
                case xJavaBlock::TYPE_TERNARY_OPERATOR:
                    UpdateBasicBlock = SearchUpdateBlockAndCreateContinueLoop(Visited, BlockPtr, BlockPtr->SecondSubBlockPtr);
                    // pass through
                case xJavaBlock::TYPE_IF:
                    if (!UpdateBasicBlock) {
                        UpdateBasicBlock = SearchUpdateBlockAndCreateContinueLoop(Visited, BlockPtr, BlockPtr->FirstSubBlockPtr);
                    }
                    if (!UpdateBasicBlock) {
                        UpdateBasicBlock = SearchUpdateBlockAndCreateContinueLoop(Visited, BlockPtr, BlockPtr->NextBlockPtr);
                    }
                    break;
                case xJavaBlock::TYPE_CONDITION_OR:
                case xJavaBlock::TYPE_CONDITION_AND:
                    UpdateBasicBlock = SearchUpdateBlockAndCreateContinueLoop(Visited, BlockPtr, BlockPtr->FirstSubBlockPtr);
                    if (!UpdateBasicBlock) {
                        UpdateBasicBlock = SearchUpdateBlockAndCreateContinueLoop(Visited, BlockPtr, BlockPtr->SecondSubBlockPtr);
                    }
                    break;
                case xJavaBlock::TYPE_SWITCH:
                    UpdateBasicBlock = SearchUpdateBlockAndCreateContinueLoop(Visited, BlockPtr, BlockPtr->NextBlockPtr);
                    // pass through
                case xJavaBlock::TYPE_SWITCH_DECLARATION:
                    for (auto & SwitchCase : BlockPtr->SwitchCases) {
                        if (!UpdateBasicBlock) {
                            UpdateBasicBlock = SearchUpdateBlockAndCreateContinueLoop(Visited, BlockPtr, SwitchCase.BlockPtr);
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        return UpdateBasicBlock;
    }

    xJavaBlock * SearchUpdateBlockAndCreateContinueLoop(xBitSet & Visited, xJavaBlock * BlockPtr, xJavaBlock * SubBlockPtr) {
        if (SubBlockPtr) {
            auto * CFGPtr = BlockPtr->GetControlFlowGraph();
            assert(CFGPtr && CFGPtr == SubBlockPtr->GetControlFlowGraph());

            if (BlockPtr->FromOffset < SubBlockPtr->FromOffset) {
                Todo();
        //         if (BlockPtr->getFirstLineNumber() == Expression.UNKNOWN_LINE_NUMBER) {
        //             if (SubBlockPtr->matchType(GROUP_SINGLE_SUCCESSOR) && (SubBlockPtr->NextBlockPtr.getType() == TYPE_LOOP_START)) {
        //                 int stackDepth = ByteCodeUtil.evalStackDepth(SubBlockPtr);

        //                 while (stackDepth != 0) {
        //                     Set<BasicBlock> predecessors = SubBlockPtr->getPredecessors();
        //                     if (predecessors.size() != 1) {
        //                         break;
        //                     }
        //                     stackDepth += ByteCodeUtil.evalStackDepth(SubBlockPtr = predecessors.iterator().next());
        //                 }

        //                 removePredecessors(SubBlockPtr);
        //                 return SubBlockPtr;
        //             }
        //         } else if (BlockPtr->getFirstLineNumber() > SubBlockPtr->getFirstLineNumber()) {
        //             removePredecessors(SubBlockPtr);
        //             return SubBlockPtr;
        //         }
            }

            return SearchUpdateBlockAndCreateContinueLoop(Visited, SubBlockPtr);
        }

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
        Fatal("Unprocessed BranchBlockPtr");
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
            auto UpdateBlockPtr = SearchUpdateBlockAndCreateContinueLoop(VisitedMembers, BlockPtr->FirstSubBlockPtr);
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

        Fatal("Unprocessed BranchBlockPtr");
        return true;
    }

    /**
     * @brief Final Reduce,
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