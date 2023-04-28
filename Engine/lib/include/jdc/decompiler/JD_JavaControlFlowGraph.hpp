#pragma once
#include "./_.hpp"
#include "./JD_JavaBlock.hpp"
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
        std::vector<xJavaBlock*>                   Blocks;
        std::vector<std::unique_ptr<xJavaBlock>>   BlockList;
        size_t                                     FirstVariableIndex;

    protected:
        const xJavaMethod *             _JavaMethodPtr;
        const xJavaClass *              _JavaClassPtr;

    protected:
        X_PRIVATE_MEMBER bool Init(const xJavaMethod * JavaMethodPtr);
        X_PRIVATE_MEMBER void InitLocalVariables();
        X_PRIVATE_MEMBER void InitBlocks();

        X_PRIVATE_MEMBER void ReduceGraph();
        X_PRIVATE_STATIC_MEMBER bool Reduce(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet);
        X_PRIVATE_STATIC_MEMBER bool ReduceConditionalBranch(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet);
        X_PRIVATE_STATIC_MEMBER bool ReduceSwitchDeclaration(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet);
        X_PRIVATE_STATIC_MEMBER bool ReduceTryDeclaration(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet);
        X_PRIVATE_STATIC_MEMBER bool ReduceJsr(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet);
        X_PRIVATE_STATIC_MEMBER bool ReduceLoop(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet);

        X_PRIVATE_STATIC_MEMBER size_t EvalStackDepth(const xJavaClass * JavaClassPtr, const std::vector<xel::ubyte> & CodeBinary, xJavaBlock * BlockPtr);

    public:
        X_PRIVATE_STATIC_MEMBER std::unique_ptr<xJavaControlFlowGraph> ParseByteCode(const xJavaMethod * JavaMethodPtr);
    };

}
