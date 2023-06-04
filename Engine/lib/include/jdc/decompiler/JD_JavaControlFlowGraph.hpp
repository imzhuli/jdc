#pragma once
#include "./_.hpp"
#include "./JD_JavaControlFlowGraph_JavaBlock.hpp"
#include "./JD_JavaControlFlowGraph_JavaLoop.hpp"
#include "./JD_JavaMethod.hpp"
#include "../base/JD_Instructions.hpp"
#include "../syntax/JD_JavaFrame.hpp"
#include "../class_file/JD_Attribute.hpp"
#include <memory>
#include <set>

namespace jdc
{
    class xJavaMethod;
    class xJavaClass;

    class xJavaControlFlowGraph;
    class xJavaBlock;
    class xJavaSwitchCase;
    class xJavaExceptionHandler;

    X_PRIVATE std::string ToString(const xJavaControlFlowGraph * CFGPtr);

    class xJavaControlFlowGraph
    : xel::xNonCopyable
    {
    public:
        std::vector<xJavaLocalVariable>            LocalVariableList;
        size_t                                     FirstVariableIndex;
        std::vector<std::unique_ptr<xJavaBlock>>   BlockList;
        xJavaBlockPtrList                   BlockPtrList; // raw pointer version of block list

    protected:
        const xJavaMethod *                        _JavaMethodPtr;
        const xJavaClass *                         _JavaClassPtr;

    public:
        X_PRIVATE_MEMBER xJavaControlFlowGraph(const xJavaMethod * JavaMethodPtr);
        X_PRIVATE_MEMBER const xJavaClass *  GetClass() const { return _JavaClassPtr; }
        X_PRIVATE_MEMBER const xJavaMethod * GetMethod() const { return _JavaMethodPtr; }
        X_PRIVATE_MEMBER const xAttributeCode * GetCodeAttribute() const { return (const xAttributeCode *)GetAttributePtr(GetMethod()->Converted.AttributeMap, "Code"); }

    protected:

        X_PRIVATE_MEMBER bool Init();
        X_PRIVATE_MEMBER void Clean();

        X_PRIVATE_MEMBER void InitLocalVariables();
        X_PRIVATE_MEMBER void InitBlocks();

        template<typename ... tArgs>
        X_INLINE xJavaBlock * NewBlock(tArgs&& ... Args) {
            auto BlockUPtr = std::make_unique<xJavaBlock>(this, std::forward<tArgs>(Args)...);
            auto BlockPtr = BlockUPtr.get();
            BlockPtr->Index = BlockList.size();
            BlockList.push_back(std::move(BlockUPtr));
            return BlockPtr;
        }

        X_INLINE xJavaBlock * CopyBlock(xJavaBlock * OriginalBlockPtr, xJavaBlockPtrSet && Predecessors = {}) {
            auto BlockUPtr = std::make_unique<xJavaBlock>(this, OriginalBlockPtr->Type, OriginalBlockPtr->FromOffset, OriginalBlockPtr->ToOffset);
            auto BlockPtr = BlockUPtr.get();
            BlockPtr->Index = BlockList.size();
            BlockList.push_back(std::move(BlockUPtr));

            BlockPtr->NextBlockPtr = OriginalBlockPtr->NextBlockPtr;
            BlockPtr->BranchBlockPtr = OriginalBlockPtr->BranchBlockPtr;
            BlockPtr->ConditionBlockPtr = OriginalBlockPtr->ConditionBlockPtr;
            BlockPtr->MustInverseCondition = OriginalBlockPtr->MustInverseCondition;
            BlockPtr->FirstSubBlockPtr = OriginalBlockPtr->FirstSubBlockPtr;
            BlockPtr->SecondSubBlockPtr = OriginalBlockPtr->SecondSubBlockPtr;
            BlockPtr->ExceptionHandlers = OriginalBlockPtr->ExceptionHandlers;
            BlockPtr->SwitchCases = OriginalBlockPtr->SwitchCases;
            BlockPtr->Predecessors = std::move(Predecessors);

            return BlockPtr;
        }

        X_INLINE xJavaBlock * CloneNextBlock(xJavaBlock * BlockPtr, xJavaBlock * NextBlockPtr) {
            assert(BlockPtr->NextBlockPtr == NextBlockPtr);
            auto Clone = NewBlock(NextBlockPtr->Type, NextBlockPtr->FromOffset, NextBlockPtr->ToOffset);
            Clone->NextBlockPtr = &xJavaBlock::End;
            Clone->Predecessors.insert(BlockPtr);
            NextBlockPtr->Predecessors.erase(NextBlockPtr->Predecessors.find(BlockPtr));
            BlockPtr->NextBlockPtr = Clone;
            return Clone;
        }

        X_INLINE xJavaBlock * NewJumpBlock(xJavaBlock * SourceBlockPtr, xJavaBlock * TargetBlockPtr) {
            auto NewBlockPtr = NewBlock(xJavaBlock::TYPE_JUMP, SourceBlockPtr->FromOffset, TargetBlockPtr->FromOffset);
            NewBlockPtr->Predecessors.insert(SourceBlockPtr);

            auto & TargetBlockPredecessors = TargetBlockPtr->Predecessors;
            TargetBlockPredecessors.erase(TargetBlockPredecessors.find(SourceBlockPtr));
            return NewBlockPtr;
        }

        X_PRIVATE_MEMBER xJavaLoop MakeLoop(xJavaBlock * StartBlockPtr, xBitSet & SearchZoneIndexes, xBitSet & MemberIndexes);
        X_PRIVATE_MEMBER std::vector<xJavaLoop> IdentifyNaturalLoops(const std::vector<xBitSet> & ArrayOfDominatorIndexes);
        X_PRIVATE_MEMBER xJavaBlock * SearchEndBasicBlock(const xBitSet & MemberIndexes, size_t MaxOffset, const xJavaBlockPtrSet & Members);
        X_PRIVATE_MEMBER void ChangeEndLoopToJump(xBitSet & Visited, xJavaBlock * TargetPtr, xJavaBlock * BlockPtr);
        X_PRIVATE_MEMBER xJavaBlock * ReduceLoop(xJavaLoop & Loop);
        X_PRIVATE_MEMBER bool ReduceLoop(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JstTargets);
        X_PRIVATE_MEMBER void ReduceGoto();
        X_PRIVATE_MEMBER void ReduceLoop();
        X_PRIVATE_MEMBER bool Reduce(xJavaBlock * BlockPtr, xBitSet & Visited, xBitSet & JstTargets);
        X_PRIVATE_MEMBER bool Reduce();

    public:
        X_PRIVATE_STATIC_MEMBER std::unique_ptr<xJavaControlFlowGraph> ParseByteCode(const xJavaMethod * JavaMethodPtr);

    private:
        friend std::string ToString(const xJavaControlFlowGraph * CFGPtr);
    };

}
