#pragma once
#include "./_.hpp"
#include "../syntax/JD_JavaType.hpp"
#include "../syntax/JD_JavaPrimitiveTypes.hpp"
#include "../syntax/JD_JavaObjectTypes.hpp"
#include <vector>
#include <set>

namespace jdc
{

    class xJavaControlFlowGraph;
    class xJavaBlock;
    class xJavaSwitchCase;
    class xJavaExceptionHander;

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

    class xJavaExceptionHander
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
        X_PRIVATE_MEMBER xJavaBlock(size_t FromOffset, size_t ToOffset)
        : xJavaBlock(TYPE_DELETED, FromOffset, ToOffset)
        {}

        X_PRIVATE_MEMBER xJavaBlock(eType Type, size_t FromOffset, size_t ToOffset)
        : Type(Type), FromOffset(FromOffset), ToOffset(ToOffset)
        {}

        X_PRIVATE_MEMBER bool Contains(xJavaBlock * CheckBlockPtr) const;
        X_PRIVATE_MEMBER void Replace(xJavaBlock * OldBlockPtr, xJavaBlock * NewBlockPtr);

    public:
        eType  Type  = TYPE_DELETED;

        size_t FromOffset = {};
        size_t ToOffset   = {};

        xJavaBlock * NextBlockPtr = {};
        xJavaBlock * BranchBlockPtr = {};
        xJavaBlock * ConditionBlockPtr = {};
        bool         InverseCondition = true;
        xJavaBlock * FirstSubBlockPtr = {};
        xJavaBlock * SecondSubBlockPtr = {};

        std::set<xJavaBlock *>              Predecessors;
        std::vector<xJavaSwitchCase>        SwitchCases;
        std::vector<xJavaExceptionHander>   ExceptionHandlers;
    };

}
