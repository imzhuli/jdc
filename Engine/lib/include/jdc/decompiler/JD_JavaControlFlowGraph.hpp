#pragma once
#include "./_.hpp"
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

    class xJavaSwitchCase
    {
    public:
        bool           DefaultCase  = {};
        size_t         Value        = {};
        size_t         Offset       = {};
        xJavaBlock *   BlockPtr     = {};

        X_PRIVATE_MEMBER xJavaSwitchCase() = default;
        X_PRIVATE_MEMBER xJavaSwitchCase(xJavaBlock * BlockPtr); // default case
        X_PRIVATE_MEMBER xJavaSwitchCase(size_t Value, xJavaBlock * BlockPtr);
    };

    class xJavaException
    {
    public:
        // size_t      Index       =  {};
        uint16_t    StartPC     =  {};
        uint16_t    EndPC       =  {};
        uint16_t    HandlerPC   =  {};
        uint16_t    CatchType   =  {};
        std::string FixedExceptionClassBinaryName;
    };

    class xJavaExceptionHandler
    {
    public:
        std::string    FixedCatchTypeName = {};
        xJavaBlock *   HandlerBlockPtr    = {};
    };

    class xJavaBlock
    {
    public:
        enum eType : uint32_t {
            TYPE_DELETED                         = 0,
            TYPE_START                           = (1 << 0),
            TYPE_END                             = (1 << 1),
            TYPE_STATEMENTS                      = (1 << 2),
            TYPE_THROW                           = (1 << 3),
            TYPE_RETURN                          = (1 << 4),
            TYPE_RETURN_VALUE                    = (1 << 5),
            TYPE_SWITCH_DECLARATION              = (1 << 6),
            TYPE_SWITCH                          = (1 << 7),
            TYPE_SWITCH_BREAK                    = (1 << 8),
            TYPE_TRY_DECLARATION                 = (1 << 9),
            TYPE_TRY                             = (1 << 10),
            TYPE_TRY_JSR                         = (1 << 11),
            TYPE_TRY_ECLIPSE                     = (1 << 12),
            TYPE_JSR                             = (1 << 13),
            TYPE_RET                             = (1 << 14),
            TYPE_CONDITIONAL_BRANCH              = (1 << 15),
            TYPE_IF                              = (1 << 16),
            TYPE_IF_ELSE                         = (1 << 17),
            TYPE_CONDITION                       = (1 << 18),
            TYPE_CONDITION_OR                    = (1 << 19),
            TYPE_CONDITION_AND                   = (1 << 20),
            TYPE_CONDITION_TERNARY_OPERATOR      = (1 << 21),
            TYPE_LOOP                            = (1 << 22),
            TYPE_LOOP_START                      = (1 << 23),
            TYPE_LOOP_CONTINUE                   = (1 << 24),
            TYPE_LOOP_END                        = (1 << 25),
            TYPE_GOTO                            = (1 << 26),
            TYPE_INFINITE_GOTO                   = (1 << 27),
            TYPE_GOTO_IN_TERNARY_OPERATOR        = (1 << 28),
            TYPE_TERNARY_OPERATOR                = (1 << 29),
            TYPE_JUMP                            = (1 << 30),

            GROUP_SINGLE_SUCCESSOR  = TYPE_START|TYPE_STATEMENTS|TYPE_TRY_DECLARATION|TYPE_JSR|TYPE_LOOP|TYPE_IF|TYPE_IF_ELSE|TYPE_SWITCH|TYPE_TRY|TYPE_TRY_JSR|TYPE_TRY_ECLIPSE|TYPE_GOTO|TYPE_GOTO_IN_TERNARY_OPERATOR|TYPE_TERNARY_OPERATOR,
            GROUP_SYNTHETIC         = TYPE_START|TYPE_END|TYPE_CONDITIONAL_BRANCH|TYPE_SWITCH_DECLARATION|TYPE_TRY_DECLARATION|TYPE_RET|TYPE_GOTO|TYPE_JUMP,
            GROUP_CODE              = TYPE_STATEMENTS|TYPE_THROW|TYPE_RETURN|TYPE_RETURN_VALUE|TYPE_SWITCH_DECLARATION|TYPE_CONDITIONAL_BRANCH|TYPE_JSR|TYPE_RET|TYPE_SWITCH|TYPE_GOTO|TYPE_INFINITE_GOTO|TYPE_GOTO_IN_TERNARY_OPERATOR|TYPE_CONDITION|TYPE_CONDITION_TERNARY_OPERATOR,
            GROUP_END               = TYPE_END|TYPE_THROW|TYPE_RETURN|TYPE_RETURN_VALUE|TYPE_RET|TYPE_SWITCH_BREAK|TYPE_LOOP_START|TYPE_LOOP_CONTINUE|TYPE_LOOP_END|TYPE_INFINITE_GOTO|TYPE_JUMP,
            GROUP_CONDITION         = TYPE_CONDITION|TYPE_CONDITION_OR|TYPE_CONDITION_AND|TYPE_CONDITION_TERNARY_OPERATOR,
        };

    public:
        X_PRIVATE_MEMBER xJavaBlock(eType Type)
        : Type(Type)
        {}

        X_PRIVATE_MEMBER xJavaBlock(const xJavaMethod * JavaMethodPtr, eType Type)
        : _JavaClassPtr(JavaMethodPtr->JavaClassPtr), _JavaMethodPtr(JavaMethodPtr), Type(Type)
        {
            auto CodeAttributePtr = (const xAttributeCode *)GetAttributePtr(_JavaMethodPtr->Converted.AttributeMap, "Code");
            assert(CodeAttributePtr);
            _CodeBinaryPtr = &CodeAttributePtr->CodeBinary;
        }

        X_PRIVATE_MEMBER xJavaBlock(const xJavaMethod * JavaMethodPtr, size_t FromOffset, size_t ToOffset)
        : xJavaBlock(JavaMethodPtr, eType::TYPE_DELETED, FromOffset, ToOffset)
        {}

        X_PRIVATE_MEMBER xJavaBlock(const xJavaMethod * JavaMethodPtr, eType Type, size_t FromOffset, size_t ToOffset)
        : _JavaClassPtr(JavaMethodPtr->JavaClassPtr), _JavaMethodPtr(JavaMethodPtr), Type(Type), FromOffset(FromOffset), ToOffset(ToOffset)
        {
            auto CodeAttributePtr = (const xAttributeCode *)GetAttributePtr(_JavaMethodPtr->Converted.AttributeMap, "Code");
            assert(CodeAttributePtr);
            _CodeBinaryPtr = &CodeAttributePtr->CodeBinary;
        }

        X_INLINE const xJavaClass * GetClassPtr() const { assert(_JavaClassPtr); return _JavaClassPtr; }
        X_INLINE const xJavaMethod * GetMethodPtr() const { assert(_JavaMethodPtr); return _JavaMethodPtr; }
        X_INLINE const std::vector<xel::ubyte> * GetCodeBinaryPtr() const { assert(_CodeBinaryPtr); return _CodeBinaryPtr; }
        X_INLINE const class xJavaControlFlowGraph * GetControlFlowGraph() const { assert(_JavaControlFlowGraphPtr); return _JavaControlFlowGraphPtr; }
        X_INLINE void  SetControlFlowGraph(class xJavaControlFlowGraph * JavaControlFlowGraphPtr) { _JavaControlFlowGraphPtr = JavaControlFlowGraphPtr; }

        X_PRIVATE_MEMBER bool Contains(xJavaBlock * CheckBlockPtr) const;
        X_PRIVATE_MEMBER void Replace(xJavaBlock * OldBlockPtr, xJavaBlock * NewBlockPtr);
        X_PRIVATE_MEMBER void AddExceptionHandler(const xJavaExceptionHandler & ExceptionHandler);
        X_PRIVATE_MEMBER void InverseCondition();


    private:
        const class xJavaClass *        _JavaClassPtr = nullptr;
        const class xJavaMethod *       _JavaMethodPtr = nullptr;
        const std::vector<xel::ubyte> * _CodeBinaryPtr = nullptr;
        class xJavaControlFlowGraph *   _JavaControlFlowGraphPtr = nullptr;

    public:
        eType  Type  = TYPE_DELETED;

        size_t FromOffset = {};
        size_t ToOffset   = {};

        xJavaBlock * NextBlockPtr = {};
        xJavaBlock * BranchBlockPtr = {};
        xJavaBlock * ConditionBlockPtr = {};
        xJavaBlock * FirstSubBlockPtr = {};
        xJavaBlock * SecondSubBlockPtr = {};
        bool         MustInverseCondition = true;

        std::set<xJavaBlock *>               Predecessors;
        std::vector<xJavaSwitchCase>         SwitchCases;
        std::vector<xJavaExceptionHandler>   ExceptionHandlers;
    };

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

        template<typename ... tArgs>
        X_INLINE xJavaBlock * NewBlock(tArgs&& ... Args) {
            auto BlockUPtr = std::make_unique<xJavaBlock>(_JavaMethodPtr, std::forward<tArgs>(Args)...);
            auto BlockPtr = BlockUPtr.get();
            BlockPtr->SetControlFlowGraph(this);
            BlockList.push_back(std::move(BlockUPtr));
            return BlockPtr;
        }

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
        X_PRIVATE_STATIC_MEMBER ssize_t  GetMinDepth(xJavaBlock * BlockPtr);
        X_PRIVATE_STATIC_MEMBER void     UpdateCondition(xJavaBlock * BlockPtr, xJavaBlock * NextNextBlockPtr, xJavaBlock * NextNextNextNextBlockPtr);
        X_PRIVATE_STATIC_MEMBER void     UpdateConditionalBranches(xJavaBlock * BlockPtr, xJavaBlock * LeftBlockPtr, xJavaBlock::eType OperatorType, xJavaBlock * SubBlockPtr);
        X_PRIVATE_STATIC_MEMBER void     UpdateConditionTernaryOperator(xJavaBlock * BlockPtr, xJavaBlock * NextNextBlockPtr);
        X_PRIVATE_STATIC_MEMBER bool     AggregateConditionalBranches(xJavaBlock * BlockPtr);

        X_PRIVATE_STATIC_MEMBER xJavaBlock EndBlock;
        X_PRIVATE_STATIC_MEMBER constexpr xJavaBlock * const EndBlockPtr = &EndBlock;

    public:
        X_PRIVATE_STATIC_MEMBER std::unique_ptr<xJavaControlFlowGraph> ParseByteCode(const xJavaMethod * JavaMethodPtr);
    };

}
