#pragma once
#include "./_.hpp"
#include "./JD_JavaBlock.hpp"
#include "../base/JD_Instructions.hpp"
#include "../syntax/JD_JavaType.hpp"
#include "../syntax/JD_JavaPrimitiveTypes.hpp"
#include "../syntax/JD_JavaObjectTypes.hpp"
#include "../syntax/JD_JavaFrame.hpp"
#include "../class_file/JD_Attribute.hpp"
#include <memory>

namespace jdc
{
    class xJavaMethod;
    class xJavaClass;

    class xJavaControlFlowGraph
    {
    public:
        std::vector<xJavaLocalVariable>            LocalVariableList;
        size_t                                     FirstVariableIndex;
        std::vector<xJavaBlock*>                   Blocks;
        std::vector<std::unique_ptr<xJavaBlock>>   BlockList;

    protected:
        const xJavaMethod *             _JavaMethodPtr;
        const xJavaClass *              _JavaClassPtr;

    protected:
        X_PRIVATE_MEMBER bool Init(const xJavaMethod * JavaMethodPtr);
        X_PRIVATE_MEMBER void Clean();

        X_PRIVATE_MEMBER void InitLocalVariables();
        X_PRIVATE_MEMBER void InitBlocks();

        X_PRIVATE_MEMBER void ReduceGraph();
        X_PRIVATE_STATIC_MEMBER bool Reduce(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet);
        X_PRIVATE_STATIC_MEMBER bool ReduceConditionalBranch(xJavaBlock * BlockPtr);
        X_PRIVATE_STATIC_MEMBER bool ReduceConditionalBranch(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet);
        X_PRIVATE_STATIC_MEMBER bool ReduceSwitchDeclaration(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet);
        X_PRIVATE_STATIC_MEMBER bool ReduceTryDeclaration(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet);
        X_PRIVATE_STATIC_MEMBER bool ReduceJsr(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet);
        X_PRIVATE_STATIC_MEMBER bool ReduceLoop(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet);

        X_PRIVATE_STATIC_MEMBER xOpCode  SearchNextOpcode(const std::vector<xel::ubyte> & CodeBinary, xJavaBlock * BlockPtr, size_t MaxOffset);
        X_PRIVATE_STATIC_MEMBER xOpCode  GetLastOpcode(const std::vector<xel::ubyte> & CodeBinary, xJavaBlock * BlockPtr);
        X_PRIVATE_STATIC_MEMBER ssize_t  EvalStackDepth(const xJavaClass * JavaClassPtr, const std::vector<xel::ubyte> & CodeBinary, xJavaBlock * BlockPtr);
        X_PRIVATE_STATIC_MEMBER ssize_t  GetMinDepth(const xJavaClass * JavaClassPtr, const std::vector<xel::ubyte> & CodeBinary, xJavaBlock * BlockPtr);
        X_PRIVATE_STATIC_MEMBER void     UpdateConditionalBranches(xJavaBlock * BlockPtr, xJavaBlock * LeftBlockPtr, xJavaBlock::eType OperatorType, xJavaBlock * SubBlockPtr);
        X_PRIVATE_STATIC_MEMBER void     UpdateConditionTernaryOperator(xJavaBlock * BlockPtr, xJavaBlock * NextNextBlockPtr);
        X_PRIVATE_STATIC_MEMBER bool     AggregateConditionalBranches(xJavaBlock * BlockPtr);

        X_PRIVATE_STATIC_MEMBER xJavaBlock EndBlock;
        X_PRIVATE_STATIC_MEMBER constexpr xJavaBlock * const EndBlockPtr = &EndBlock;

    public:
        X_PRIVATE_STATIC_MEMBER std::unique_ptr<xJavaControlFlowGraph> ParseByteCode(const xJavaMethod * JavaMethodPtr);
    };

}
