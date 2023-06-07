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

    static bool ContainsFinally(xJavaBlock * BlockPtr);
    static bool CheckEclipseFinallyPattern(xJavaBlock * BlockPtr, xJavaBlock * FinallyBlockPtr, size_t MaxOffset);
    static xJavaBlock * UpdateBlock(xJavaBlock * BlockPtr, xJavaBlock * EndBlockPtr, size_t MaxOffset);
    static xJavaBlock * SearchJsrTarget(xJavaBlock * BlockPtr, xBitSet & JsrTargets);
    static xJavaBlock * GetLastConditionalBranch(xJavaBlock * BlockPtr, xBitSet & Visited);
    static void RemoveJsrAndMergeSubTry(xJavaBlock * BlockPtr);
    static void RemoveLastContinueLoop(xJavaBlock * BlockPtr);

    namespace {

        class xWatchDogLink
        {
        protected:
            size_t ParentIndex;
            size_t ChildIndex;

        public:
            xWatchDogLink(xJavaBlock * ParentBlockPtr, xJavaBlock * ChildBlockPtr) {
                ParentIndex = ParentBlockPtr->Index;
                ChildIndex = ChildBlockPtr->Index;
            }
            bool operator < (const xWatchDogLink & Other) const {
                if (ParentIndex < Other.ParentIndex) {
                    return true;
                }
                if (ParentIndex == Other.ParentIndex) {
                    return ChildIndex < Other.ChildIndex;
                }
                return false;
            }
        };

        X_INLINE bool operator == (const xWatchDogLink & lhs, const xWatchDogLink & rhs) {
            return !(lhs < rhs) && !(rhs < lhs);
        }

        X_INLINE bool operator != (const xWatchDogLink & lhs, const xWatchDogLink & rhs) {
            return (lhs < rhs) || (rhs < lhs);
        }

        class xWatchDog {
        protected:
            std::set<xWatchDogLink> Links;

        public:
            void Clear() {
                Links.clear();
            }

            void Check(xJavaBlock * ParentBlockPtr, xJavaBlock * ChildBlockPtr) { // check loop or multiple entry
                if (!(ChildBlockPtr->Type & xJavaBlock::GROUP_END)) {
                    auto Link = xWatchDogLink(ParentBlockPtr, ChildBlockPtr);
                    if (Links.find(Link) != Links.end()) {
                        X_DEBUG_PRINTF("CFG watchdog: parent=%zi, child=%zi\n" + ParentBlockPtr->Index, ChildBlockPtr->Index);
                        Fatal();
                    }
                    Links.insert(Link);
                }
            }

        };
    }

    void xJavaControlFlowGraph::UpdateConditionTernaryOperator(xJavaBlock * BlockPtr, xJavaBlock * NextNextBlockPtr)
    {
        auto FromOffset =  NextNextBlockPtr->FromOffset;
        auto ToOffset = NextNextBlockPtr->ToOffset;
        auto NextNextNextBlockPtr = NextNextBlockPtr->NextBlockPtr;
        auto NextNextBranchBlockPtr = NextNextBlockPtr->BranchBlockPtr;

        if (BlockPtr->Type == xJavaBlock::TYPE_CONDITIONAL_BRANCH) {
            BlockPtr->Type = xJavaBlock::TYPE_CONDITION;
        }
        if ((NextNextBlockPtr->Type == xJavaBlock::TYPE_CONDITION) && !NextNextBlockPtr->MustInverseCondition) {
            BlockPtr->InverseCondition();
        }

        auto ConditionBlockPtr = NextNextBlockPtr;

        ConditionBlockPtr->Type = BlockPtr->Type;
        ConditionBlockPtr->FromOffset = BlockPtr->FromOffset;
        ConditionBlockPtr->ToOffset = BlockPtr->ToOffset;
        ConditionBlockPtr->NextBlockPtr = &xJavaBlock::End;
        ConditionBlockPtr->BranchBlockPtr = &xJavaBlock::End;
        ConditionBlockPtr->ConditionBlockPtr = BlockPtr->ConditionBlockPtr;
        ConditionBlockPtr->FirstSubBlockPtr = BlockPtr->FirstSubBlockPtr;
        ConditionBlockPtr->SecondSubBlockPtr = BlockPtr->SecondSubBlockPtr;
        ConditionBlockPtr->Predecessors.clear();

        BlockPtr->Type = xJavaBlock::TYPE_CONDITION_TERNARY_OPERATOR;
        BlockPtr->FromOffset = FromOffset;
        BlockPtr->ToOffset = ToOffset;
        BlockPtr->ConditionBlockPtr = ConditionBlockPtr;
        BlockPtr->FirstSubBlockPtr = BlockPtr->NextBlockPtr;
        BlockPtr->SecondSubBlockPtr = BlockPtr->BranchBlockPtr;
        BlockPtr->NextBlockPtr = NextNextNextBlockPtr;
        BlockPtr->BranchBlockPtr = NextNextBranchBlockPtr;
        BlockPtr->FirstSubBlockPtr->NextBlockPtr = &xJavaBlock::End;
        BlockPtr->SecondSubBlockPtr->NextBlockPtr = &xJavaBlock::End;

        NextNextNextBlockPtr->Replace(NextNextBlockPtr, BlockPtr);
        NextNextBranchBlockPtr->Replace(NextNextBlockPtr, BlockPtr);

        BlockPtr->FirstSubBlockPtr->Predecessors.clear();
        BlockPtr->SecondSubBlockPtr->Predecessors.clear();
    }

    void xJavaControlFlowGraph::UpdateConditionTernaryOperator(xJavaBlock * BlockPtr) {
        auto NextBlockPtr = BlockPtr->NextBlockPtr;
        auto BranchBlockPtr = BlockPtr->BranchBlockPtr;

        auto cfg = BlockPtr->GetControlFlowGraph();
        auto ConditionBlockPtr = cfg->NewBlock(xJavaBlock::TYPE_CONDITION, BlockPtr->FromOffset, BlockPtr->ToOffset);

        ConditionBlockPtr->NextBlockPtr = &xJavaBlock::End;
        ConditionBlockPtr->BranchBlockPtr = &xJavaBlock::End;

        BlockPtr->Type = xJavaBlock::TYPE_CONDITION_TERNARY_OPERATOR;
        BlockPtr->ToOffset = BlockPtr->FromOffset;
        BlockPtr->ConditionBlockPtr = ConditionBlockPtr;
        BlockPtr->FirstSubBlockPtr = NextBlockPtr;
        BlockPtr->SecondSubBlockPtr = BranchBlockPtr;
        BlockPtr->NextBlockPtr = NextBlockPtr->NextBlockPtr;
        BlockPtr->BranchBlockPtr = NextBlockPtr->BranchBlockPtr;

        NextBlockPtr->NextBlockPtr->Replace(NextBlockPtr, BlockPtr);
        NextBlockPtr->BranchBlockPtr->Replace(NextBlockPtr, BlockPtr);
        BranchBlockPtr->NextBlockPtr->Replace(BranchBlockPtr, BlockPtr);
        BranchBlockPtr->BranchBlockPtr->Replace(BranchBlockPtr, BlockPtr);

        NextBlockPtr->Predecessors.clear();
        BranchBlockPtr->Predecessors.clear();
    }

    void xJavaControlFlowGraph::UpdateCondition(xJavaBlock * BlockPtr, xJavaBlock * NextNextBlockPtr, xJavaBlock * NextNextNextNextBlockPtr)
    {
        auto FromOffset =  NextNextNextNextBlockPtr->FromOffset;
        auto ToOffset = NextNextNextNextBlockPtr->ToOffset;
        auto NextBlockPtr = NextNextNextNextBlockPtr->NextBlockPtr;
        auto BranchBlockPtr = NextNextNextNextBlockPtr->BranchBlockPtr;

        auto ConditionBlockPtr = CopyBlock(BlockPtr);
        ConditionBlockPtr->Type = xJavaBlock::TYPE_CONDITION;

        BlockPtr->NextBlockPtr->NextBlockPtr = &xJavaBlock::End;
        BlockPtr->NextBlockPtr->Predecessors.clear();
        BlockPtr->BranchBlockPtr->NextBlockPtr = &xJavaBlock::End;
        BlockPtr->BranchBlockPtr->Predecessors.clear();

        NextNextNextNextBlockPtr->Type = xJavaBlock::TYPE_CONDITION_TERNARY_OPERATOR;
        NextNextNextNextBlockPtr->FromOffset = ConditionBlockPtr->ToOffset; // same as to ?
        NextNextNextNextBlockPtr->ToOffset = ConditionBlockPtr->ToOffset;
        NextNextNextNextBlockPtr->ConditionBlockPtr = ConditionBlockPtr;
        NextNextNextNextBlockPtr->FirstSubBlockPtr = BlockPtr->NextBlockPtr;
        NextNextNextNextBlockPtr->SecondSubBlockPtr = BlockPtr->BranchBlockPtr;
        NextNextNextNextBlockPtr->NextBlockPtr = &xJavaBlock::End;
        NextNextNextNextBlockPtr->BranchBlockPtr = &xJavaBlock::End;
        ConditionBlockPtr->NextBlockPtr = &xJavaBlock::End;
        ConditionBlockPtr->BranchBlockPtr = &xJavaBlock::End;

        ConditionBlockPtr = CopyBlock(NextNextBlockPtr);
        ConditionBlockPtr->Type = xJavaBlock::TYPE_CONDITION;

        NextNextBlockPtr->NextBlockPtr->NextBlockPtr = &xJavaBlock::End;
        NextNextBlockPtr->NextBlockPtr->Predecessors.clear();
        NextNextBlockPtr->BranchBlockPtr->NextBlockPtr = &xJavaBlock::End;
        NextNextBlockPtr->BranchBlockPtr->Predecessors.clear();

        NextNextBlockPtr->Type = xJavaBlock::TYPE_CONDITION_TERNARY_OPERATOR;
        NextNextBlockPtr->FromOffset = ConditionBlockPtr->ToOffset;
        NextNextBlockPtr->ToOffset = ConditionBlockPtr->ToOffset;
        NextNextBlockPtr->ConditionBlockPtr = ConditionBlockPtr;
        NextNextBlockPtr->FirstSubBlockPtr = NextNextBlockPtr->NextBlockPtr;
        NextNextBlockPtr->SecondSubBlockPtr = NextNextBlockPtr->BranchBlockPtr;
        NextNextBlockPtr->NextBlockPtr = &xJavaBlock::End;
        NextNextBlockPtr->BranchBlockPtr = &xJavaBlock::End;
        ConditionBlockPtr->NextBlockPtr = &xJavaBlock::End;
        ConditionBlockPtr->BranchBlockPtr = &xJavaBlock::End;

        BlockPtr->Type = xJavaBlock::TYPE_CONDITION;
        BlockPtr->FromOffset = FromOffset;
        BlockPtr->ToOffset = ToOffset;
        BlockPtr->FirstSubBlockPtr = NextNextNextNextBlockPtr;
        BlockPtr->SecondSubBlockPtr = NextNextBlockPtr;
        BlockPtr->NextBlockPtr = NextBlockPtr;
        BlockPtr->BranchBlockPtr = BranchBlockPtr;

        NextBlockPtr->Replace(NextNextNextNextBlockPtr, BlockPtr);
        BranchBlockPtr->Replace(NextNextNextNextBlockPtr, BlockPtr);
    }

    bool CheckJdk118TernaryOperatorPattern(xJavaBlock * NextBlockPtr, xJavaBlock * NextNextBlockPtr, xOpCode IfByteCode) {
        if ((NextNextBlockPtr->ToOffset - NextNextBlockPtr->FromOffset) == 3) {
            assert(NextBlockPtr->GetCode() == NextNextBlockPtr->GetCode());
            auto & Code = * NextBlockPtr->GetCode();
            auto NextFromOffset = NextBlockPtr->FromOffset;
            auto NextNextFromOffset = NextNextBlockPtr->FromOffset;
            return (Code[NextFromOffset] == 3) &&                                                               // ICONST_0
                    (((Code[NextFromOffset + 1] & 255) == 167) || ((Code[NextFromOffset + 1] & 255) == 200)) && // GOTO or GOTO_W
                    ((Code[NextNextFromOffset] & 255) == IfByteCode) &&                                         // IFEQ or IFNE
                    (NextNextFromOffset + 3 == NextNextBlockPtr->ToOffset);
        }
        return false;
    }

    void ConvertConditionalBranchToGotoInTernaryOperator(xJavaBlock * BlockPtr, xJavaBlock * NextBlockPtr, xJavaBlock * NextNextBlockPtr)
    {
        // TODO
    }

    void ConvertGotoInTernaryOperatorToCondition(xJavaBlock * BlockPtr, xJavaBlock * NextBlockPtr)
    {
        BlockPtr->Type = xJavaBlock::TYPE_CONDITION;
        BlockPtr->NextBlockPtr = NextBlockPtr->NextBlockPtr;
        BlockPtr->BranchBlockPtr = NextBlockPtr->BranchBlockPtr;

        NextBlockPtr->NextBlockPtr->Replace(NextBlockPtr, BlockPtr);
        NextBlockPtr->BranchBlockPtr->Replace(NextBlockPtr, BlockPtr);

        NextBlockPtr->Type = xJavaBlock::TYPE_DELETED;
    }

    void xJavaControlFlowGraph::UpdateConditionalBranches(xJavaBlock * BlockPtr, xJavaBlock * LeftBlockPtr, xJavaBlock::eType OpType, xJavaBlock * SubBlockPtr)
    {
        BlockPtr->Type = OpType;
        BlockPtr->ToOffset = SubBlockPtr->ToOffset;
        BlockPtr->NextBlockPtr = SubBlockPtr->NextBlockPtr;
        BlockPtr->BranchBlockPtr = SubBlockPtr->BranchBlockPtr;
        BlockPtr->ConditionBlockPtr = &xJavaBlock::End;
        BlockPtr->FirstSubBlockPtr = LeftBlockPtr;
        BlockPtr->SecondSubBlockPtr = SubBlockPtr;

        SubBlockPtr->NextBlockPtr->Replace(SubBlockPtr, BlockPtr);
        SubBlockPtr->BranchBlockPtr->Replace(SubBlockPtr, BlockPtr);
    }

    xJavaBlock * xJavaControlFlowGraph::CreateLeftCondition(xJavaBlock * BlockPtr) {
        if (BlockPtr->Type == xJavaBlock::TYPE_CONDITIONAL_BRANCH) {
            return NewBlock(xJavaBlock::TYPE_CONDITION, BlockPtr->FromOffset, BlockPtr->ToOffset, false);
        }
        auto LeftBlockPtr = CopyBlock(BlockPtr);
        LeftBlockPtr->InverseCondition();
        return LeftBlockPtr;
    }

    xJavaBlock * xJavaControlFlowGraph::CreateLeftInverseCondition(xJavaBlock * BlockPtr) {
        if (BlockPtr->Type == xJavaBlock::TYPE_CONDITIONAL_BRANCH) {
            return NewBlock(xJavaBlock::TYPE_CONDITION, BlockPtr->FromOffset, BlockPtr->ToOffset);
        }
        return CopyBlock(BlockPtr);
    }

    bool xJavaControlFlowGraph::AggregateConditionalBranches(xJavaBlock * BlockPtr)
    {
        auto Change = false;

        auto NextBlockPtr = BlockPtr->NextBlockPtr;
        auto BranchBlockPtr = BlockPtr->BranchBlockPtr;

        if ((NextBlockPtr->Type == xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR) && (NextBlockPtr->Predecessors.size() == 1)) {
            auto NextNextBlockPtr = NextBlockPtr->NextBlockPtr;

            if (NextNextBlockPtr->Type & (xJavaBlock::TYPE_CONDITIONAL_BRANCH | xJavaBlock::TYPE_CONDITION)) {
                if ((BranchBlockPtr->Type & (xJavaBlock::TYPE_STATEMENTS | xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR)) &&
                    (NextNextBlockPtr == BranchBlockPtr->NextBlockPtr) &&
                    (BranchBlockPtr->Predecessors.size() == 1) &&
                    (NextNextBlockPtr->Predecessors.size() == 2)) {

                    if (NextNextBlockPtr->GetMinDepth() == -1) {
                        UpdateConditionTernaryOperator(BlockPtr, NextNextBlockPtr);
                        return true;
                    }

                    auto NextNextNextBlockPtr = NextNextBlockPtr->NextBlockPtr;
                    auto NextNextBranchBlockPtr = NextNextBlockPtr->BranchBlockPtr;

                    if ((NextNextNextBlockPtr->Type == xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR) && (NextNextNextBlockPtr->Predecessors.size() == 1)) {
                        auto NextNextNextNextBlockPtr = NextNextNextBlockPtr->NextBlockPtr;

                        if (NextNextNextNextBlockPtr->Type & (xJavaBlock::TYPE_CONDITIONAL_BRANCH | xJavaBlock::TYPE_CONDITION)) {
                            if ((NextNextBranchBlockPtr->Type & (xJavaBlock::TYPE_STATEMENTS | xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR)) &&
                                (NextNextNextNextBlockPtr == NextNextBranchBlockPtr->NextBlockPtr) &&
                                (NextNextBranchBlockPtr->Predecessors.size() == 1) &&
                                (NextNextNextNextBlockPtr->Predecessors.size() == 2)) {

                                if (NextNextNextNextBlockPtr->GetMinDepth() == -2) {
                                    UpdateCondition(BlockPtr, NextNextBlockPtr, NextNextNextNextBlockPtr);
                                    return true;
                                }
                            }
                        }
                    }
                }
                if ((NextNextBlockPtr->NextBlockPtr == BranchBlockPtr) && CheckJdk118TernaryOperatorPattern(NextBlockPtr, NextNextBlockPtr, xOpCode(153))) { // IFEQ
                    ConvertConditionalBranchToGotoInTernaryOperator(BlockPtr, NextBlockPtr, NextNextBlockPtr);
                    return true;
                }
                if ((NextNextBlockPtr->BranchBlockPtr == BranchBlockPtr) && CheckJdk118TernaryOperatorPattern(NextBlockPtr, NextNextBlockPtr, 154)) { // IFNE
                    ConvertConditionalBranchToGotoInTernaryOperator(BlockPtr, NextBlockPtr, NextNextBlockPtr);
                    return true;
                }
                if (NextNextBlockPtr->Predecessors.size() == 1) {
                    ConvertGotoInTernaryOperatorToCondition(NextBlockPtr, NextNextBlockPtr);
                    return true;
                }
            }
        }

        if (NextBlockPtr->Type & (xJavaBlock::TYPE_CONDITIONAL_BRANCH | xJavaBlock::GROUP_CONDITION)) {
            // Test line numbers
            auto LineNumber1 = GetLastLineNumber(BlockPtr);
            auto LineNumber2 = GetFirstLineNumber(NextBlockPtr);
            assert(LineNumber1 <= LineNumber2);

            if ((LineNumber2-LineNumber1) <= 1) {
                Change = AggregateConditionalBranches(NextBlockPtr);
                if ((NextBlockPtr->Type & (xJavaBlock::TYPE_CONDITIONAL_BRANCH | xJavaBlock::GROUP_CONDITION)) &&
                    (NextBlockPtr->Predecessors.size() == 1)) {
                    // Aggregate conditional branches
                    if (NextBlockPtr->NextBlockPtr == BranchBlockPtr) {
                        UpdateConditionalBranches(BlockPtr, CreateLeftCondition(BlockPtr), xJavaBlock::TYPE_CONDITION_OR, NextBlockPtr);
                        return true;
                    }
                    else if (NextBlockPtr->BranchBlockPtr == BranchBlockPtr) {
                        UpdateConditionalBranches(BlockPtr, CreateLeftInverseCondition(BlockPtr), xJavaBlock::TYPE_CONDITION_AND, NextBlockPtr);
                        return true;
                    }
                    else if (BranchBlockPtr->Type & (xJavaBlock::TYPE_CONDITIONAL_BRANCH | xJavaBlock::GROUP_CONDITION)) {
                        Change = AggregateConditionalBranches(BranchBlockPtr);
                        if (BranchBlockPtr->Type & (xJavaBlock::TYPE_CONDITIONAL_BRANCH | xJavaBlock::GROUP_CONDITION)) {
                            if ((NextBlockPtr->NextBlockPtr == BranchBlockPtr->NextBlockPtr) && (NextBlockPtr->BranchBlockPtr == BranchBlockPtr->BranchBlockPtr)) {
                                UpdateConditionTernaryOperator(BlockPtr);
                                return true;
                            } else if ((NextBlockPtr->BranchBlockPtr == BranchBlockPtr->NextBlockPtr) && (NextBlockPtr->NextBlockPtr == BranchBlockPtr->BranchBlockPtr)) {
                                UpdateConditionTernaryOperator(BlockPtr);
                                BranchBlockPtr->InverseCondition();
                                return true;
                            }
                        }
                    }
                }
            }
        }

        if (BranchBlockPtr->Type & (xJavaBlock::TYPE_CONDITIONAL_BRANCH | xJavaBlock::GROUP_CONDITION)) {
            // Test line numbers
            auto LineNumber1 = GetLastLineNumber(BlockPtr);
            auto LineNumber2 = GetFirstLineNumber(BranchBlockPtr);
            assert(LineNumber1 <= LineNumber2);

            if ((LineNumber2-LineNumber1) <= 1) {
                Change = AggregateConditionalBranches(BranchBlockPtr);

                if (BranchBlockPtr->Type & (xJavaBlock::TYPE_CONDITIONAL_BRANCH | xJavaBlock::GROUP_CONDITION) && (BranchBlockPtr->Predecessors.size() == 1)) {
                    // Aggregate conditional branches
                    if (BranchBlockPtr->BranchBlockPtr == NextBlockPtr) {
                        UpdateConditionalBranches(BlockPtr, CreateLeftCondition(BlockPtr), xJavaBlock::TYPE_CONDITION_AND, BranchBlockPtr);
                        return true;
                    } else if (BranchBlockPtr->NextBlockPtr == NextBlockPtr) {
                        UpdateConditionalBranches(BlockPtr, CreateLeftInverseCondition(BlockPtr), xJavaBlock::TYPE_CONDITION_OR, BranchBlockPtr);
                        return true;
                    }
                }
            }
        }

        if (BlockPtr->Type == xJavaBlock::TYPE_CONDITIONAL_BRANCH) {
            BlockPtr->Type = xJavaBlock::TYPE_CONDITION;
            return true;
        }

        return Change;
    }

    bool xJavaControlFlowGraph::ReduceConditionalBranch(xJavaBlock * BlockPtr)
    {
        // TODO
        return false;
    }

    bool xJavaControlFlowGraph::ReduceConditionalBranch(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JsrTargets)
    {
        while (AggregateConditionalBranches(BlockPtr))
        {}

        assert(BlockPtr->Type & xJavaBlock::GROUP_CONDITION);
        if (Reduce(BlockPtr->NextBlockPtr, Visited, JsrTargets) & Reduce(BlockPtr->BranchBlockPtr, Visited, JsrTargets)) {
            return ReduceConditionalBranch(BlockPtr);
        }

        return false;
    }

    void Visit(xJavaBlock * BlockPtr, xBitSet & Visited, size_t MaxOffset, xJavaBlockPtrSet & Ends) {
        if (BlockPtr->FromOffset >= MaxOffset) {
            Ends.insert(BlockPtr);
        }
        else if ((BlockPtr->Index >= 0) && (!Visited[BlockPtr->Index])) {
            Visited[BlockPtr->Index] = true;
            switch (BlockPtr->Type) {
                case xJavaBlock::TYPE_CONDITIONAL_BRANCH:
                case xJavaBlock::TYPE_JSR:
                case xJavaBlock::TYPE_CONDITION:
                    Visit(BlockPtr->BranchBlockPtr, Visited, MaxOffset, Ends);
                case xJavaBlock::TYPE_START:
                case xJavaBlock::TYPE_STATEMENTS:
                case xJavaBlock::TYPE_GOTO:
                case xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR:
                case xJavaBlock::TYPE_LOOP:
                    Visit(BlockPtr->NextBlockPtr, Visited, MaxOffset, Ends);
                    break;
                case xJavaBlock::TYPE_TRY:
                case xJavaBlock::TYPE_TRY_JSR:
                case xJavaBlock::TYPE_TRY_ECLIPSE:
                    Visit(BlockPtr->FirstSubBlockPtr, Visited, MaxOffset, Ends);
                case xJavaBlock::TYPE_TRY_DECLARATION:
                    for (auto & ExceptionHandler : BlockPtr->ExceptionHandlers) {
                        Visit(ExceptionHandler.BlockPtr, Visited, MaxOffset, Ends);
                    }
                    Visit(BlockPtr->NextBlockPtr, Visited, MaxOffset, Ends);
                    break;
                case xJavaBlock::TYPE_IF_ELSE:
                case xJavaBlock::TYPE_TERNARY_OPERATOR:
                    Visit(BlockPtr->SecondSubBlockPtr, Visited, MaxOffset, Ends);
                case xJavaBlock::TYPE_IF:
                    Visit(BlockPtr->FirstSubBlockPtr, Visited, MaxOffset, Ends);
                    Visit(BlockPtr->NextBlockPtr, Visited, MaxOffset, Ends);
                    break;
                case xJavaBlock::TYPE_CONDITION_OR:
                case xJavaBlock::TYPE_CONDITION_AND:
                    Visit(BlockPtr->FirstSubBlockPtr, Visited, MaxOffset, Ends);
                    Visit(BlockPtr->SecondSubBlockPtr, Visited, MaxOffset, Ends);
                    break;
                case xJavaBlock::TYPE_SWITCH:
                    Visit(BlockPtr->NextBlockPtr, Visited, MaxOffset, Ends);
                case xJavaBlock::TYPE_SWITCH_DECLARATION:
                    for (auto & SwitchCase : BlockPtr->SwitchCases) {
                        Visit(SwitchCase.BlockPtr, Visited, MaxOffset, Ends);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    bool SearchLoopStart(xJavaBlock * BlockPtr, size_t MaxOffset) {
        auto WatchDog = xWatchDog();

        for (auto & SwitchCase : BlockPtr->SwitchCases) {
            WatchDog.Clear();
            auto SwitchCaseBlockPtr = SwitchCase.BlockPtr;

            while (SwitchCaseBlockPtr->FromOffset < MaxOffset) {
                if (SwitchCaseBlockPtr == &xJavaBlock::LoopStart) {
                    return true;
                }

                if (SwitchCaseBlockPtr->Type & (xJavaBlock::GROUP_END | xJavaBlock::GROUP_CONDITION)) {
                    break;
                }

                auto NextBlockPtr = xJavaBlockPtr(nullptr);

                if (SwitchCaseBlockPtr->Type & xJavaBlock::GROUP_SINGLE_SUCCESSOR) {
                    NextBlockPtr = SwitchCaseBlockPtr->NextBlockPtr;
                } else if (SwitchCaseBlockPtr->Type == xJavaBlock::TYPE_CONDITIONAL_BRANCH) {
                    NextBlockPtr = SwitchCaseBlockPtr->BranchBlockPtr;
                } else if (SwitchCaseBlockPtr->Type == xJavaBlock::TYPE_SWITCH_DECLARATION) {
                    auto Max = SwitchCaseBlockPtr->FromOffset;
                    for (auto & SubSC : SwitchCaseBlockPtr->SwitchCases) {
                        if (Max < SubSC.BlockPtr->FromOffset) {
                            NextBlockPtr = SubSC.BlockPtr;
                            Max = NextBlockPtr->FromOffset;
                        }
                    }
                }

                if (SwitchCaseBlockPtr == NextBlockPtr) {
                    break;
                }

                WatchDog.Check(SwitchCaseBlockPtr, NextBlockPtr);
                SwitchCaseBlockPtr = NextBlockPtr;
            }
        }

        return false;
    }

     void ReplaceLoopStartWithSwitchBreak(xJavaBlock * BlockPtr, xBitSet & Visited)
     {
        if (!(BlockPtr->Type & xJavaBlock::GROUP_END) && (!Visited[BlockPtr->Index])) {
            Visited[BlockPtr->Index] = true;
            BlockPtr->Replace(&xJavaBlock::LoopStart, &xJavaBlock::SwitchBreak);

            switch (BlockPtr->Type) {
                case xJavaBlock::TYPE_CONDITIONAL_BRANCH:
                case xJavaBlock::TYPE_JSR:
                case xJavaBlock::TYPE_CONDITION:
                    ReplaceLoopStartWithSwitchBreak(BlockPtr->BranchBlockPtr, Visited);
                case xJavaBlock::TYPE_START:
                case xJavaBlock::TYPE_STATEMENTS:
                case xJavaBlock::TYPE_GOTO:
                case xJavaBlock::TYPE_GOTO_IN_TERNARY_OPERATOR:
                case xJavaBlock::TYPE_LOOP:
                    ReplaceLoopStartWithSwitchBreak(BlockPtr->NextBlockPtr, Visited);
                    break;
                case xJavaBlock::TYPE_TRY:
                case xJavaBlock::TYPE_TRY_JSR:
                case xJavaBlock::TYPE_TRY_ECLIPSE:
                    ReplaceLoopStartWithSwitchBreak(BlockPtr->FirstSubBlockPtr, Visited);
                case xJavaBlock::TYPE_TRY_DECLARATION:
                    for (auto & ExceptionHandler : BlockPtr->ExceptionHandlers) {
                        ReplaceLoopStartWithSwitchBreak(ExceptionHandler.BlockPtr, Visited);
                    }
                    break;
                case xJavaBlock::TYPE_IF_ELSE:
                case xJavaBlock::TYPE_TERNARY_OPERATOR:
                    ReplaceLoopStartWithSwitchBreak(BlockPtr->SecondSubBlockPtr, Visited);
                case xJavaBlock::TYPE_IF:
                    ReplaceLoopStartWithSwitchBreak(BlockPtr->FirstSubBlockPtr, Visited);
                    ReplaceLoopStartWithSwitchBreak(BlockPtr->NextBlockPtr, Visited);
                    break;
                case xJavaBlock::TYPE_CONDITION_OR:
                case xJavaBlock::TYPE_CONDITION_AND:
                    ReplaceLoopStartWithSwitchBreak(BlockPtr->FirstSubBlockPtr, Visited);
                    ReplaceLoopStartWithSwitchBreak(BlockPtr->SecondSubBlockPtr, Visited);
                    break;
                case xJavaBlock::TYPE_SWITCH:
                    ReplaceLoopStartWithSwitchBreak(BlockPtr->NextBlockPtr, Visited);
                case xJavaBlock::TYPE_SWITCH_DECLARATION:
                    for (auto & SwitchCase : BlockPtr->SwitchCases) {
                        ReplaceLoopStartWithSwitchBreak(SwitchCase.BlockPtr, Visited);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    bool xJavaControlFlowGraph::ReduceSwitchDeclaration(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JsrTargets)
    {
        xJavaSwitchCase * DefaultSCPtr = nullptr;
        xJavaSwitchCase * LastSCPtr = nullptr;
        size_t MaxOffset = 0;

        for (auto & SwitchCase : BlockPtr->SwitchCases) {
            if (MaxOffset < SwitchCase.Offset) {
                MaxOffset = SwitchCase.Offset;
            }

            if (SwitchCase.IsDefaultCase) {
                DefaultSCPtr = &SwitchCase;
            } else {
                LastSCPtr = &SwitchCase;
            }
        }

        if (!LastSCPtr) {
            LastSCPtr = DefaultSCPtr;
        }

        auto LastSwitchCaseBasicBlock = xJavaBlockPtr();
        auto V = xBitSet();
        auto Ends = xJavaBlockPtrSet();

        for (auto & SwitchCase : BlockPtr->SwitchCases) {
            auto SwitchCaseBlockPtr = SwitchCase.BlockPtr;

            if (SwitchCase.Offset == MaxOffset) {
                LastSwitchCaseBasicBlock = SwitchCaseBlockPtr;
            } else {
                Visit(SwitchCaseBlockPtr, V, MaxOffset, Ends);
            }
        }

        auto EndBlockPtr = &xJavaBlock::End;

        for (auto TempBlockPtr : Ends) {
            if ((EndBlockPtr == &xJavaBlock::End) || (EndBlockPtr->FromOffset < TempBlockPtr->FromOffset)) {
                EndBlockPtr = TempBlockPtr;
            }
        }

        if (EndBlockPtr == &xJavaBlock::End) {
            if ((LastSCPtr->BlockPtr == LastSwitchCaseBasicBlock) && SearchLoopStart(BlockPtr, MaxOffset)) {
                ReplaceLoopStartWithSwitchBreak(BlockPtr, X2Ref(xBitSet()));
                DefaultSCPtr->BlockPtr = (EndBlockPtr = &xJavaBlock::LoopStart);
            } else {
                EndBlockPtr = LastSwitchCaseBasicBlock;
            }
        } else {
            Visit(LastSwitchCaseBasicBlock, V, EndBlockPtr->FromOffset, Ends);
        }

        auto & EndPredecessors = EndBlockPtr->Predecessors;
        for (auto Iter = EndPredecessors.begin() ; Iter != EndPredecessors.end();) {
            auto & EndPrecedessor = *Iter;
            if (V[EndPrecedessor->Index]) {
                EndPrecedessor->Replace(EndBlockPtr, &xJavaBlock::SwitchBreak);
                Iter = EndPredecessors.erase(Iter);
            } else {
                ++Iter;
            }
        }

        if (DefaultSCPtr->BlockPtr == EndBlockPtr) {
            auto & SwitchCases = BlockPtr->SwitchCases;
            for (auto Iter = SwitchCases.begin(); Iter != SwitchCases.end();) {
                if (Iter->BlockPtr == EndBlockPtr) {
                    Iter = SwitchCases.erase(Iter);
                }
            }
        } else {
            for (auto & SwitchCase : BlockPtr->SwitchCases) {
                if (SwitchCase.BlockPtr == EndBlockPtr) {
                    SwitchCase.BlockPtr = &xJavaBlock::SwitchBreak;
                }
            }
        }

        auto Reduced = true;
        for (auto & SwitchCase : BlockPtr->SwitchCases) {
            Reduced &= Reduce(SwitchCase.BlockPtr, Visited, JsrTargets);
        }

        for (auto & SwitchCase : BlockPtr->SwitchCases) {
            auto SwitchBlockPtr = SwitchCase.BlockPtr;
            assert(SwitchBlockPtr != EndBlockPtr);

            auto & Predecessors = SwitchBlockPtr->Predecessors;
            if (Predecessors.size() > 1) {
                for (auto Iter = Predecessors.begin(); Iter != Predecessors.end();) {
                    auto & Predecessor = *Iter;
                    if (Predecessor != BlockPtr) {
                        Predecessor->Replace(SwitchBlockPtr, &xJavaBlock::End);
                        Predecessors.erase(Iter);
                    }
                }
            }
        }

        // // Change type
        BlockPtr->Type = xJavaBlock::TYPE_SWITCH;
        BlockPtr->NextBlockPtr = EndBlockPtr;
        EndPredecessors.insert(BlockPtr);

        return Reduced & Reduce(BlockPtr->NextBlockPtr, Visited, JsrTargets);
    }

    bool xJavaControlFlowGraph::ReduceTryDeclaration(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JsrTargets)
    {
        auto Reduced = true;
        auto FinallyBlockPtr = xJavaBlockPtr();

        for (auto & ExceptionHandler : BlockPtr->ExceptionHandlers) {
            if (ExceptionHandler.FixedCatchTypeName.empty()) {
                Reduced = Reduce(ExceptionHandler.BlockPtr, Visited, JsrTargets);
                FinallyBlockPtr = ExceptionHandler.BlockPtr;
                break;
            }
        }

        auto JsrTarget = SearchJsrTarget(BlockPtr, JsrTargets);
        Reduced &= Reduce(BlockPtr->NextBlockPtr, Visited, JsrTargets);

        auto TryBlockPtr = BlockPtr->NextBlockPtr;
        if (TryBlockPtr->Type & xJavaBlock::GROUP_SYNTHETIC) {
            return false;
        }

        auto MaxOffset = BlockPtr->FromOffset;
        auto TryWithResourcesFlag = true;
        auto TryWithResourcesBlockPtr = xJavaBlockPtr();

        for (auto & ExceptionHandler : BlockPtr->ExceptionHandlers) {
            if (!ExceptionHandler.FixedCatchTypeName.empty()) {
                Reduced &= Reduce(ExceptionHandler.BlockPtr, Visited, JsrTargets);
            }

            auto ExceptionBlockPtr = ExceptionHandler.BlockPtr;
            if (ExceptionBlockPtr->Type & xJavaBlock::GROUP_SYNTHETIC) {
                return false;
            }

            if (MaxOffset < ExceptionBlockPtr->FromOffset) {
                MaxOffset = ExceptionBlockPtr->FromOffset;
            }

            if (TryWithResourcesFlag) {
                auto & Predecessors = ExceptionBlockPtr->Predecessors;
                if (Predecessors.size() == 1) {
                    TryWithResourcesFlag = false;
                } else {
                    assert(Predecessors.size() == 2);
                    if (!TryWithResourcesBlockPtr) {
                        for (auto & PredecessorBlockPtr : Predecessors) {
                            if (PredecessorBlockPtr != BlockPtr) {
                                assert(PredecessorBlockPtr->Type == xJavaBlock::TYPE_TRY_DECLARATION);
                                TryWithResourcesBlockPtr = PredecessorBlockPtr;
                                break;
                            }
                        }
                    } else if (Predecessors.find(TryWithResourcesBlockPtr) == Predecessors.end()) {
                        TryWithResourcesFlag = false;
                    }
                }
            }
        }

        if (TryWithResourcesFlag) {
            // One of 'try-with-resources' patterns
            for (auto & ExceptionHandler : BlockPtr->ExceptionHandlers) {
                ExceptionHandler.BlockPtr->Predecessors.erase(ExceptionHandler.BlockPtr->Predecessors.find(BlockPtr));
            }
            for (auto & PredecessorBlockPtr : BlockPtr->Predecessors) {
                PredecessorBlockPtr->Replace(BlockPtr, TryBlockPtr);
                TryBlockPtr->Replace(BlockPtr, PredecessorBlockPtr);
            }
            BlockPtr->Type = xJavaBlock::TYPE_DELETED;
        }
        else if (Reduced) {
            auto EndBlockPtr = SearchEndBlock(BlockPtr, MaxOffset);
            UpdateBlock(TryBlockPtr, EndBlockPtr, MaxOffset);

            if ((FinallyBlockPtr) &&
                (BlockPtr->ExceptionHandlers.size() == 1) &&
                (TryBlockPtr->Type == xJavaBlock::TYPE_TRY) &&
                (TryBlockPtr->NextBlockPtr == &xJavaBlock::End) &&
                (BlockPtr->FromOffset == TryBlockPtr->FromOffset) &&
                !ContainsFinally(TryBlockPtr)) {
                // Merge inner try
                BlockPtr->ExceptionHandlers.insert(BlockPtr->ExceptionHandlers.begin(), TryBlockPtr->ExceptionHandlers.begin(), TryBlockPtr->ExceptionHandlers.end());

                for (auto & ExceptionHandler : TryBlockPtr->ExceptionHandlers) {
                    auto & ExceptionHandlerPredecessors = ExceptionHandler.BlockPtr->Predecessors;
                    ExceptionHandlerPredecessors.clear();
                    ExceptionHandlerPredecessors.insert(BlockPtr);
                }

                TryBlockPtr->Type = xJavaBlock::TYPE_DELETED;
                TryBlockPtr = TryBlockPtr->FirstSubBlockPtr;
                auto & TryBlockPredecessors = TryBlockPtr->Predecessors;
                TryBlockPredecessors.clear();
                TryBlockPredecessors.insert(BlockPtr);
            }

            // Update blocks
            size_t ToOffset = MaxOffset;

            for (auto & ExceptionHandler : BlockPtr->ExceptionHandlers) {
                auto ExceptionHandlerBlockPtr = ExceptionHandler.BlockPtr;

                if (ExceptionHandlerBlockPtr == EndBlockPtr) {
                    ExceptionHandler.BlockPtr = &xJavaBlock::End;
                } else {
                    auto Offset = (ExceptionHandlerBlockPtr->FromOffset == MaxOffset) ? EndBlockPtr->FromOffset : MaxOffset;

                    if (Offset == 0) {
                        Offset = SIZE_MAX;
                    }
                    auto LastBlockPtr = UpdateBlock(ExceptionHandlerBlockPtr, EndBlockPtr, Offset);

                    if (ToOffset < LastBlockPtr->ToOffset) {
                        ToOffset = LastBlockPtr->ToOffset;
                    }
                }
            }

            BlockPtr->FirstSubBlockPtr = TryBlockPtr;
            BlockPtr->NextBlockPtr = EndBlockPtr;
            EndBlockPtr->Predecessors.insert(BlockPtr);

            if (JsrTarget) {
                // Change type
                if ((FinallyBlockPtr) && CheckEclipseFinallyPattern(BlockPtr, FinallyBlockPtr, MaxOffset)) {
                    BlockPtr->Type = xJavaBlock::TYPE_TRY_ECLIPSE;
                } else {
                    BlockPtr->Type = xJavaBlock::TYPE_TRY;
                }
            } else {
                // Change type
                BlockPtr->Type = xJavaBlock::TYPE_TRY_JSR;
                // Merge 1.1 to 1.4 sub try block
                RemoveJsrAndMergeSubTry(BlockPtr);
            }

            BlockPtr->ToOffset = ToOffset;
        }

        return Reduced;
    }

    bool xJavaControlFlowGraph::ReduceJsr(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JsrTargets)
    {
        auto BranchBlockPtr = BlockPtr->BranchBlockPtr;
        auto Reduced = Reduce(BlockPtr->NextBlockPtr, Visited, JsrTargets) & Reduce(BranchBlockPtr, Visited, JsrTargets);

        if ((BranchBlockPtr->Index >= 0) && JsrTargets[BranchBlockPtr->Index]) {
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
            // Iterator<auto> iterator = BlockPtr->BranchBlockPtr->Predecessors.iterator();
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

    void RemoveJsrAndMergeSubTry(xJavaBlock * BlockPtr)
    {
        if (BlockPtr->ExceptionHandlers.size() == 1) {
            auto SubTryBlockPtr = BlockPtr->FirstSubBlockPtr;

            if (SubTryBlockPtr->Type & (xJavaBlock::TYPE_TRY | xJavaBlock::TYPE_TRY_JSR | xJavaBlock::TYPE_TRY_ECLIPSE)) {
                for (auto & ExceptionHandler : SubTryBlockPtr->ExceptionHandlers) {
                    if (ExceptionHandler.FixedCatchTypeName.empty())
                        return;
                }

                // Append 'catch' handlers
                for (auto & ExceptionHandler : SubTryBlockPtr->ExceptionHandlers) {
                    auto ExceptionHandlerBlockPtr = ExceptionHandler.BlockPtr;
                    BlockPtr->ExceptionHandlers.push_back(xJavaExceptionHandler{ ExceptionHandler.FixedCatchTypeName, ExceptionHandlerBlockPtr});
                    ExceptionHandlerBlockPtr->Replace(SubTryBlockPtr, BlockPtr);
                }

                // Move 'try' clause to parent 'try' block
                BlockPtr->FirstSubBlockPtr = SubTryBlockPtr->FirstSubBlockPtr;
                SubTryBlockPtr->FirstSubBlockPtr->Replace(SubTryBlockPtr, BlockPtr);
            }
        }
    }

    bool ContainsFinally(xJavaBlock * BlockPtr)
    {
        for (auto & ExceptionHandler : BlockPtr->ExceptionHandlers) {
            if (ExceptionHandler.FixedCatchTypeName.empty()) {
                return true;
            }
        }
        return false;
    }

    bool CheckEclipseFinallyPattern(xJavaBlock * BlockPtr, xJavaBlock * FinallyBlockPtr, size_t MaxOffset)
    {
        auto NextOpcode = BlockPtr->GetNextOpCode(MaxOffset);
        if ((NextOpcode == 0)   ||
            (NextOpcode == 167) || // GOTO
            (NextOpcode == 200)) { // GOTO_W
            return true;
        }

        auto NextBlockPtr = BlockPtr->NextBlockPtr;
        if (!(NextBlockPtr->Type & xJavaBlock::GROUP_END) && (FinallyBlockPtr->FromOffset < NextBlockPtr->FromOffset)) {
            auto CFGPtr = FinallyBlockPtr->GetControlFlowGraph();
            auto ToLineNumber   = CFGPtr->GetLineNumber(FinallyBlockPtr->ToOffset - 1);
            auto FromLineNumber = CFGPtr->GetLineNumber(NextBlockPtr->FromOffset);

            if (FromLineNumber < ToLineNumber) {
                return true;
            }
        }

        return false;
    }

    xJavaBlock * UpdateBlock(xJavaBlock * BlockPtr, xJavaBlock * EndBlockPtr, size_t MaxOffset)
    {
        auto Watchdog = xWatchDog();
        while (BlockPtr->Type & xJavaBlock::GROUP_SINGLE_SUCCESSOR) {
            Watchdog.Check(BlockPtr, BlockPtr->NextBlockPtr);
            auto NextBlockPtr = BlockPtr->NextBlockPtr;
            if ((NextBlockPtr == EndBlockPtr) || (NextBlockPtr->FromOffset > MaxOffset)) {
                NextBlockPtr->Predecessors.erase(NextBlockPtr->Predecessors.find(BlockPtr));
                BlockPtr->NextBlockPtr = &xJavaBlock::End;
                break;
            }
            BlockPtr = NextBlockPtr;
        }
        return BlockPtr;
    }

    xJavaBlock * SearchJsrTarget(xJavaBlock * BlockPtr, xBitSet & JsrTargets)
    {
        for (auto & ExceptionHandler : BlockPtr->ExceptionHandlers) {
            if (ExceptionHandler.FixedCatchTypeName.empty()) {
                auto ExceptionBlockPtr = ExceptionHandler.BlockPtr;

                if (ExceptionBlockPtr->Type == xJavaBlock::TYPE_STATEMENTS) {
                    ExceptionBlockPtr = ExceptionBlockPtr->NextBlockPtr;

                    if ((ExceptionBlockPtr->Type == xJavaBlock::TYPE_JSR) && (ExceptionBlockPtr->NextBlockPtr->Type == xJavaBlock::TYPE_THROW)) {
                        // Java 1.1 to 1.4 finally pattern found
                        auto JsrTargetBlockPtr = ExceptionBlockPtr->BranchBlockPtr;
                        JsrTargets[JsrTargetBlockPtr->Index];
                        return JsrTargetBlockPtr;
                    }
                }
            }
        }

        return nullptr;
    }

    xJavaBlock * SplitSequence(xJavaBlock * BlockPtr, size_t MaxOffset)
    {
        auto NextBlockPtr = BlockPtr->NextBlockPtr;
        auto WatchDog = xWatchDog();

        while ((NextBlockPtr->FromOffset < MaxOffset) && (NextBlockPtr->Type & xJavaBlock::GROUP_SINGLE_SUCCESSOR)) {
            WatchDog.Check(NextBlockPtr, NextBlockPtr->NextBlockPtr);
            BlockPtr = NextBlockPtr;
            NextBlockPtr = NextBlockPtr->NextBlockPtr;
        }

        if ((BlockPtr->ToOffset > MaxOffset) && (BlockPtr->Type == xJavaBlock::TYPE_TRY)) {
            // Split last try block
            auto & ExceptionHandlers = BlockPtr->ExceptionHandlers;
            auto ExceptionHandlerBlockPtr = ExceptionHandlers.back().BlockPtr;
            auto LastBlockPtr = SplitSequence(ExceptionHandlerBlockPtr, MaxOffset);

            NextBlockPtr = LastBlockPtr->NextBlockPtr;
            LastBlockPtr->NextBlockPtr = &xJavaBlock::End;

            BlockPtr->ToOffset = LastBlockPtr->ToOffset;
            BlockPtr->NextBlockPtr = NextBlockPtr;

            NextBlockPtr->Predecessors.erase(NextBlockPtr->Predecessors.find(LastBlockPtr));
            NextBlockPtr->Predecessors.insert(BlockPtr);
        }

        return BlockPtr;
    }

    xJavaBlock * xJavaControlFlowGraph::SearchEndBlock(xJavaBlock * BlockPtr, size_t MaxOffset)
    {
        auto EndBlockPtr  = xJavaBlockPtr(nullptr);
        auto LastBlockPtr = SplitSequence(BlockPtr->NextBlockPtr, MaxOffset);

        if (!(LastBlockPtr->Type & xJavaBlock::GROUP_END)) {
            auto NextBlockPtr = LastBlockPtr->NextBlockPtr;
            if ((NextBlockPtr->FromOffset >= MaxOffset) ||
                (!(NextBlockPtr->Type & (xJavaBlock::TYPE_END | xJavaBlock::TYPE_RETURN | xJavaBlock::TYPE_SWITCH_BREAK | xJavaBlock::TYPE_LOOP_START | xJavaBlock::TYPE_LOOP_CONTINUE | xJavaBlock::TYPE_LOOP_END)) && (NextBlockPtr->ToOffset < BlockPtr->FromOffset))
                ) {
                return NextBlockPtr;
            }

            EndBlockPtr = NextBlockPtr;
        }

        for (auto & ExceptionHandler : BlockPtr->ExceptionHandlers) {
            auto ExceptionHandlerBlockPtr = ExceptionHandler.BlockPtr;

            if (ExceptionHandlerBlockPtr->FromOffset < MaxOffset) {
                LastBlockPtr = SplitSequence(ExceptionHandlerBlockPtr, MaxOffset);

                if (!(LastBlockPtr->Type & xJavaBlock::GROUP_END)) {
                    auto NextBlockPtr = LastBlockPtr->NextBlockPtr;
                    if ((NextBlockPtr->FromOffset >= MaxOffset)
                        || (!(NextBlockPtr->Type & (xJavaBlock::TYPE_END | xJavaBlock::TYPE_RETURN | xJavaBlock::TYPE_SWITCH_BREAK | xJavaBlock::TYPE_LOOP_START | xJavaBlock::TYPE_LOOP_CONTINUE | xJavaBlock::TYPE_LOOP_END))
                            && (NextBlockPtr->ToOffset < BlockPtr->FromOffset))
                        ) {
                        return NextBlockPtr;
                    }

                    if (!EndBlockPtr) {
                        EndBlockPtr = NextBlockPtr;
                    } else if (EndBlockPtr != NextBlockPtr) {
                        EndBlockPtr = &xJavaBlock::End;
                    }
                }
            }
            else {
                // Last handler block
                auto LineNumber = GetLineNumber(ExceptionHandlerBlockPtr->FromOffset);
                auto WatchDog = xWatchDog();
                auto NextBlockPtr = ExceptionHandlerBlockPtr->NextBlockPtr;

                LastBlockPtr = ExceptionHandlerBlockPtr;
                while ((LastBlockPtr != NextBlockPtr) && (LastBlockPtr->Type & xJavaBlock::GROUP_SINGLE_SUCCESSOR) && (NextBlockPtr->Predecessors.size() == 1)
                    && (LineNumber <= GetLineNumber(NextBlockPtr->FromOffset))
                    ) {
                    WatchDog.Check(NextBlockPtr, NextBlockPtr->NextBlockPtr);
                    LastBlockPtr = NextBlockPtr;
                    NextBlockPtr = NextBlockPtr->NextBlockPtr;
                }

                if (!(LastBlockPtr->Type & xJavaBlock::GROUP_END)) {
                    if ((LastBlockPtr != NextBlockPtr) && ((NextBlockPtr->Predecessors.size() > 1) || !(NextBlockPtr->Type & xJavaBlock::GROUP_END))) {
                        return NextBlockPtr;
                    }

                    if ((EndBlockPtr != NextBlockPtr) && (!ExceptionHandler.FixedCatchTypeName.empty())) {
                        EndBlockPtr = &xJavaBlock::End;
                    }
                }
            }
        }

        if (EndBlockPtr && (EndBlockPtr->Type & xJavaBlock::TYPE_SWITCH_BREAK | xJavaBlock::TYPE_LOOP_START | xJavaBlock::TYPE_LOOP_CONTINUE | xJavaBlock::TYPE_LOOP_END)) {
            return EndBlockPtr;
        }

        return &xJavaBlock::End;
    }

    xJavaBlock * xJavaControlFlowGraph::SearchUpdateBlockAndCreateContinueLoop(xBitSet & Visited, xJavaBlock * BlockPtr)
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

    static void RemovePredecessors(xJavaBlock * BlockPtr) {
        auto & Predecessors = BlockPtr->Predecessors;
        for (auto & Predecessor : Predecessors) {
            Predecessor->Replace(BlockPtr, &xJavaBlock::LoopContinue);
        }
        Predecessors.clear();
    }

    xJavaBlock * xJavaControlFlowGraph::SearchUpdateBlockAndCreateContinueLoop(xBitSet & Visited, xJavaBlock * BlockPtr, xJavaBlock * SubBlockPtr)
    {
        if (SubBlockPtr) {
            auto * CFGPtr = BlockPtr->GetControlFlowGraph();
            assert(CFGPtr && CFGPtr == SubBlockPtr->GetControlFlowGraph());

            if (BlockPtr->FromOffset < SubBlockPtr->FromOffset) {
                if (GetFirstLineNumber(BlockPtr) == UNKNOWN_LINE_NUMBER) {
                    if ((SubBlockPtr->Type & xJavaBlock::GROUP_SINGLE_SUCCESSOR) && (SubBlockPtr->NextBlockPtr->Type == xJavaBlock::TYPE_LOOP_START)) {
                        auto StackDepth = SubBlockPtr->EvalStackDepth();
                        while (StackDepth != 0) {
                            auto & Predecessors = SubBlockPtr->Predecessors;
                            if (Predecessors.size() != 1) {
                                break;
                            }
                            auto Iter = Predecessors.begin();
                            SubBlockPtr = *Iter;
                            StackDepth += SubBlockPtr->EvalStackDepth();
                        }

                        RemovePredecessors(SubBlockPtr);
                        return SubBlockPtr;
                    }
                } else if (GetFirstLineNumber(BlockPtr) > GetFirstLineNumber(SubBlockPtr)) {
                    RemovePredecessors(SubBlockPtr);
                    return SubBlockPtr;
                }
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

    bool xJavaControlFlowGraph::ReduceLoop(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JsrTargets)
    {
        auto VisitedBackup = Visited;
        auto Reduced = Reduce(BlockPtr->FirstSubBlockPtr, Visited, JsrTargets);

        if (!Reduced) {
            Visited = VisitedBackup; // restore
            auto VisitedMembers = xBitSet();
            auto UpdateBlockPtr = SearchUpdateBlockAndCreateContinueLoop(VisitedMembers, BlockPtr->FirstSubBlockPtr);
            Reduced = Reduce(BlockPtr->FirstSubBlockPtr, Visited, JsrTargets);

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
                    Reduced = Reduce(NewLoopBlockPtr, VisitedMembers, JsrTargets);
                }
            }
        }

        return Reduced & Reduce(BlockPtr->NextBlockPtr, Visited, JsrTargets);
    }

    bool xJavaControlFlowGraph::Reduce(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JsrTargets)
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
                return Reduce(BlockPtr->NextBlockPtr, Visited, JsrTargets);
            case xJavaBlock::TYPE_CONDITIONAL_BRANCH:
            case xJavaBlock::TYPE_CONDITION:
            case xJavaBlock::TYPE_CONDITION_OR:
            case xJavaBlock::TYPE_CONDITION_AND:
            case xJavaBlock::TYPE_CONDITION_TERNARY_OPERATOR:
                return ReduceConditionalBranch(BlockPtr, Visited, JsrTargets);
            case xJavaBlock::TYPE_SWITCH_DECLARATION:
                return ReduceSwitchDeclaration(BlockPtr, Visited, JsrTargets);
            case xJavaBlock::TYPE_TRY_DECLARATION:
                return ReduceTryDeclaration(BlockPtr, Visited, JsrTargets);
            case xJavaBlock::TYPE_JSR:
                return ReduceJsr(BlockPtr, Visited, JsrTargets);
            case xJavaBlock::TYPE_LOOP:
                return ReduceLoop(BlockPtr, Visited, JsrTargets);
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
        auto Visited = xBitSet(BlockPtrList.size());
        auto JsrTargets = xBitSet();

        return Reduce(StartBlockPtr, Visited, JsrTargets);
    }


}