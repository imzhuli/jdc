#pragma once
#include "./_.hpp"
#include "../syntax/JD_JavaType.hpp"
#include "../syntax/JD_JavaPrimitiveTypes.hpp"
#include "../syntax/JD_JavaObjectTypes.hpp"

namespace jdc
{

    class xJavaControlFlowGraph;
    class xJavaBlock;
    class xJavaSwitchCase;
    class xJavaExceptionHander;

    class xJavaSwitchCase
    {

    };

    class xJavaBlock
    {
    public:
        using xType = uint32_t;

        static constexpr const xType TYPE_DELETED                         = 0;
        static constexpr const xType TYPE_START                           = (1 << 0);
        static constexpr const xType TYPE_END                             = (1 << 1);
        static constexpr const xType TYPE_STATEMENTS                      = (1 << 2);
        static constexpr const xType TYPE_THROW                           = (1 << 3);
        static constexpr const xType TYPE_RETURN                          = (1 << 4);
        static constexpr const xType TYPE_RETURN_VALUE                    = (1 << 5);
        static constexpr const xType TYPE_SWITCH_DECLARATION              = (1 << 6);
        static constexpr const xType TYPE_SWITCH                          = (1 << 7);
        static constexpr const xType TYPE_SWITCH_BREAK                    = (1 << 8);
        static constexpr const xType TYPE_TRY_DECLARATION                 = (1 << 9);
        static constexpr const xType TYPE_TRY                             = (1 << 10);
        static constexpr const xType TYPE_TRY_JSR                         = (1 << 11);
        static constexpr const xType TYPE_TRY_ECLIPSE                     = (1 << 12);
        static constexpr const xType TYPE_JSR                             = (1 << 13);
        static constexpr const xType TYPE_RET                             = (1 << 14);
        static constexpr const xType TYPE_CONDITIONAL_BRANCH              = (1 << 15);
        static constexpr const xType TYPE_IF                              = (1 << 16);
        static constexpr const xType TYPE_IF_ELSE                         = (1 << 17);
        static constexpr const xType TYPE_CONDITION                       = (1 << 18);
        static constexpr const xType TYPE_CONDITION_OR                    = (1 << 19);
        static constexpr const xType TYPE_CONDITION_AND                   = (1 << 20);
        static constexpr const xType TYPE_CONDITION_TERNARY_OPERATOR      = (1 << 21);
        static constexpr const xType TYPE_LOOP                            = (1 << 22);
        static constexpr const xType TYPE_LOOP_START                      = (1 << 23);
        static constexpr const xType TYPE_LOOP_CONTINUE                   = (1 << 24);
        static constexpr const xType TYPE_LOOP_END                        = (1 << 25);
        static constexpr const xType TYPE_GOTO                            = (1 << 26);
        static constexpr const xType TYPE_INFINITE_GOTO                   = (1 << 27);
        static constexpr const xType TYPE_GOTO_IN_TERNARY_OPERATOR        = (1 << 28);
        static constexpr const xType TYPE_TERNARY_OPERATOR                = (1 << 29);
        static constexpr const xType TYPE_JUMP                            = (1 << 30);

        static constexpr const xType GROUP_SINGLE_SUCCESSOR  = TYPE_START|TYPE_STATEMENTS|TYPE_TRY_DECLARATION|TYPE_JSR|TYPE_LOOP|TYPE_IF|TYPE_IF_ELSE|TYPE_SWITCH|TYPE_TRY|TYPE_TRY_JSR|TYPE_TRY_ECLIPSE|TYPE_GOTO|TYPE_GOTO_IN_TERNARY_OPERATOR|TYPE_TERNARY_OPERATOR;
        static constexpr const xType GROUP_SYNTHETIC         = TYPE_START|TYPE_END|TYPE_CONDITIONAL_BRANCH|TYPE_SWITCH_DECLARATION|TYPE_TRY_DECLARATION|TYPE_RET|TYPE_GOTO|TYPE_JUMP;
        static constexpr const xType GROUP_CODE              = TYPE_STATEMENTS|TYPE_THROW|TYPE_RETURN|TYPE_RETURN_VALUE|TYPE_SWITCH_DECLARATION|TYPE_CONDITIONAL_BRANCH|TYPE_JSR|TYPE_RET|TYPE_SWITCH|TYPE_GOTO|TYPE_INFINITE_GOTO|TYPE_GOTO_IN_TERNARY_OPERATOR|TYPE_CONDITION|TYPE_CONDITION_TERNARY_OPERATOR;
        static constexpr const xType GROUP_END               = TYPE_END|TYPE_THROW|TYPE_RETURN|TYPE_RETURN_VALUE|TYPE_RET|TYPE_SWITCH_BREAK|TYPE_LOOP_START|TYPE_LOOP_CONTINUE|TYPE_LOOP_END|TYPE_INFINITE_GOTO|TYPE_JUMP;
        static constexpr const xType GROUP_CONDITION         = TYPE_CONDITION|TYPE_CONDITION_OR|TYPE_CONDITION_AND|TYPE_CONDITION_TERNARY_OPERATOR;


    public:
        xJavaControlFlowGraph * ControlFlowGraphPtr = nullptr;
        size_t Index = {};
        xType  Type  = TYPE_DELETED;

        size_t FromOffset = {};
        size_t ToOffset   = {};

        xJavaBlock * NextBlockPtr = {};
        xJavaBlock * BranchBlockPtr = {};
        xJavaBlock * ConditionBlockPtr = {};
        bool         InverseCondition = false;
        xJavaBlock * SubBlockPtr1 = {};
        xJavaBlock * SubBlockPtr2 = {};

        std::vector<xJavaExceptionHander *> ExceptionHandlerPtrs;
        std::vector<xJavaSwitchCase *>      SwitchCases;
        std::vector<xJavaBlock *>           Predecessors;

    };



}
