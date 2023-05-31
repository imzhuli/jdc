#pragma once
#include "./_.hpp"
#include "./JD_JavaControlFlowGraph_JavaBlock.hpp"
#include "./JD_JavaMethod.hpp"
#include "../base/JD_Instructions.hpp"
#include "../syntax/JD_JavaType.hpp"
#include "../syntax/JD_JavaPrimitiveTypes.hpp"
#include "../syntax/JD_JavaObjectTypes.hpp"
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
        std::vector<xJavaBlock*>                   Blocks;
        std::vector<std::unique_ptr<xJavaBlock>>   BlockList;

        std::unique_ptr<xJavaBlock>                EndBlockUPtr;
        xJavaBlock *                               EndBlockPtr;

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

        // X_PRIVATE_MEMBER void ReduceGraph();
        // X_PRIVATE_STATIC_MEMBER bool Reduce(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet);
        // X_PRIVATE_STATIC_MEMBER bool ReduceConditionalBranch(xJavaBlock * BlockPtr);
        // X_PRIVATE_STATIC_MEMBER bool ReduceConditionalBranch(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet);
        // X_PRIVATE_STATIC_MEMBER bool ReduceSwitchDeclaration(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet);
        // X_PRIVATE_STATIC_MEMBER bool ReduceTryDeclaration(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet);
        // X_PRIVATE_STATIC_MEMBER bool ReduceJsr(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet);
        // X_PRIVATE_STATIC_MEMBER bool ReduceLoop(xJavaBlock * BlockPtr, std::set<xJavaBlock*> & VisitedSet,  std::set<xJavaBlock*> & JsrTargetSet);

        // X_PRIVATE_STATIC_MEMBER void          UpdateCondition(xJavaBlock * BlockPtr, xJavaBlock * NextNextBlockPtr, xJavaBlock * NextNextNextNextBlockPtr);
        // X_PRIVATE_STATIC_MEMBER void          UpdateConditionalBranches(xJavaBlock * BlockPtr, xJavaBlock * LeftBlockPtr, xJavaBlock::eType OperatorType, xJavaBlock * SubBlockPtr);
        // X_PRIVATE_STATIC_MEMBER void          UpdateConditionTernaryOperator(xJavaBlock * BlockPtr, xJavaBlock * NextNextBlockPtr);
        // X_PRIVATE_STATIC_MEMBER bool          AggregateConditionalBranches(xJavaBlock * BlockPtr);

    public:
        X_PRIVATE_STATIC_MEMBER std::unique_ptr<xJavaControlFlowGraph> ParseByteCode(const xJavaMethod * JavaMethodPtr);

    private:
        friend std::string ToString(const xJavaControlFlowGraph * CFGPtr);
    };

}
