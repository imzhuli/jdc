#include <jdc/decompiler/JD_JavaControlFlowGraph.hpp>
#include <jdc/decompiler/JD_JavaMethod.hpp>
#include <jdc/decompiler/JD_JavaClass.hpp>
#include <jdc/decompiler/JD_JavaSpace.hpp>
#include <xel/String.hpp>
#include <algorithm>
#include <sstream>

using namespace std;

namespace jdc
{

    using namespace std::literals::string_literals;


    size_t GetNumberOfTrailingZeros(int32_t i) {
        // HD, Count trailing 0's
        i = ~i & (i - 1);
        if (i <= 0) return i & 32;
        int n = 1;
        if (i > 1 << 16) { n += 16; i = ((uint32_t)i) >> 16; }
        if (i > 1 <<  8) { n +=  8; i = ((uint32_t)i) >>  8; }
        if (i > 1 <<  4) { n +=  4; i = ((uint32_t)i) >>  4; }
        if (i > 1 <<  2) { n +=  2; i = ((uint32_t)i) >>  2; }
        return n + (i = ((uint32_t)i) >> 1);
    }

    std::string ToString(const xJavaBlock * BlockPtr)
    {
        auto OS = std::ostringstream();
        OS << "BasicBlock{index=" << BlockPtr->Index
            << ", from=" << BlockPtr->FromOffset
            << ", to=" << BlockPtr->ToOffset
            << ", type=" << (ToString(BlockPtr->Type).c_str() + 5)
            << ", inverseCondition=" << BlockPtr->MustInverseCondition;

        OS << "}";
        return OS.str();
    }

    #define BLOCK_TYPE_TO_STRING(x) case (xJavaBlock::x): return #x##s
    std::string ToString(const xJavaBlock::eType Type)
    {
        switch(Type) {
            BLOCK_TYPE_TO_STRING(TYPE_DELETED);
            BLOCK_TYPE_TO_STRING(TYPE_START);
            BLOCK_TYPE_TO_STRING(TYPE_END);
            BLOCK_TYPE_TO_STRING(TYPE_STATEMENTS);
            BLOCK_TYPE_TO_STRING(TYPE_THROW);
            BLOCK_TYPE_TO_STRING(TYPE_RETURN);
            BLOCK_TYPE_TO_STRING(TYPE_RETURN_VALUE);
            BLOCK_TYPE_TO_STRING(TYPE_SWITCH_DECLARATION);
            BLOCK_TYPE_TO_STRING(TYPE_SWITCH);
            BLOCK_TYPE_TO_STRING(TYPE_SWITCH_BREAK);
            BLOCK_TYPE_TO_STRING(TYPE_TRY_DECLARATION);
            BLOCK_TYPE_TO_STRING(TYPE_TRY);
            BLOCK_TYPE_TO_STRING(TYPE_TRY_JSR);
            BLOCK_TYPE_TO_STRING(TYPE_TRY_ECLIPSE);
            BLOCK_TYPE_TO_STRING(TYPE_JSR);
            BLOCK_TYPE_TO_STRING(TYPE_RET);
            BLOCK_TYPE_TO_STRING(TYPE_CONDITIONAL_BRANCH);
            BLOCK_TYPE_TO_STRING(TYPE_IF);
            BLOCK_TYPE_TO_STRING(TYPE_IF_ELSE);
            BLOCK_TYPE_TO_STRING(TYPE_CONDITION);
            BLOCK_TYPE_TO_STRING(TYPE_CONDITION_OR);
            BLOCK_TYPE_TO_STRING(TYPE_CONDITION_AND);
            BLOCK_TYPE_TO_STRING(TYPE_CONDITION_TERNARY_OPERATOR);
            BLOCK_TYPE_TO_STRING(TYPE_LOOP);
            BLOCK_TYPE_TO_STRING(TYPE_LOOP_START);
            BLOCK_TYPE_TO_STRING(TYPE_LOOP_CONTINUE);
            BLOCK_TYPE_TO_STRING(TYPE_LOOP_END);
            BLOCK_TYPE_TO_STRING(TYPE_GOTO);
            BLOCK_TYPE_TO_STRING(TYPE_INFINITE_GOTO);
            BLOCK_TYPE_TO_STRING(TYPE_GOTO_IN_TERNARY_OPERATOR);
            BLOCK_TYPE_TO_STRING(TYPE_TERNARY_OPERATOR);
            BLOCK_TYPE_TO_STRING(TYPE_JUMP);
        default:
            break;
        }
        return "TYPE_INVALID"s;
    };

    xJavaBlock::xJavaBlock(xJavaControlFlowGraph * CFGPtr, eType Type, size_t FromOffset, size_t ToOffset)
    : _JavaControlFlowGraphPtr(CFGPtr), Type(Type), FromOffset(FromOffset), ToOffset(ToOffset)
    {
        auto CodeAttributePtr = (const xAttributeCode *)GetAttributePtr(GetMethod()->Converted.AttributeMap, "Code");
        assert(CodeAttributePtr);
        _CodeBinaryPtr = &CodeAttributePtr->CodeBinary;
    }

    const xJavaClass * xJavaBlock::GetClass() const
    {
        return _JavaControlFlowGraphPtr->GetClass();
    }

    const xJavaMethod * xJavaBlock::GetMethod() const
    {
        return _JavaControlFlowGraphPtr->GetMethod();
    }

    bool xJavaBlock::Contains(xJavaBlock * CheckBlockPtr) const
    {
        if (NextBlockPtr == CheckBlockPtr) {
            return true;
        }
        if (BranchBlockPtr == CheckBlockPtr) {
            return true;
        }
        for (auto & EH : ExceptionHandlers) {
            if (EH.HandlerBlockPtr == CheckBlockPtr) {
                return true;
            }
        }
        for (auto & SC : SwitchCases) {
            if (SC.BlockPtr == CheckBlockPtr) {
                return true;
            }
        }
        if (FirstSubBlockPtr == CheckBlockPtr) {
            return true;
        }
        if (SecondSubBlockPtr == CheckBlockPtr) {
            return true;
        }
        return false;
    }

    void xJavaBlock::Replace(xJavaBlock * OldBlockPtr, xJavaBlock * NewBlockPtr)
    {
        assert(OldBlockPtr);
        assert(NewBlockPtr);
        if (NextBlockPtr == OldBlockPtr) {
            NextBlockPtr = NewBlockPtr;
        }

        if (BranchBlockPtr == OldBlockPtr) {
            BranchBlockPtr = NewBlockPtr;
        }
        for (auto & EH : ExceptionHandlers) {
            EH.HandlerBlockPtr->Replace(OldBlockPtr, NewBlockPtr);
        }
        for (auto & SC : SwitchCases) {
            SC.BlockPtr->Replace(OldBlockPtr, NewBlockPtr);
        }
        if (FirstSubBlockPtr == OldBlockPtr) {
            FirstSubBlockPtr = NewBlockPtr;
        }
        if (SecondSubBlockPtr == OldBlockPtr) {
            SecondSubBlockPtr = NewBlockPtr;
        }

        auto Iter = Predecessors.find(OldBlockPtr);
        if (Iter != Predecessors.end()) {
            Predecessors.erase(Iter);
            if (NewBlockPtr->Type != xJavaBlock::TYPE_END) {
                Predecessors.insert(NewBlockPtr);
            }
        }
    }

    void xJavaBlock::AddExceptionHandler(const xJavaExceptionHandler & ExceptionHandler)
    {
    #ifndef NDEBUG
        for(auto & Handler : ExceptionHandlers) {
            if (Handler.FixedCatchTypeName == ExceptionHandler.FixedCatchTypeName) {
                xel::Fatal("Duplicate Exception catch type");
            }
        }
    #endif
        ExceptionHandlers.push_back(ExceptionHandler);
    }

    void xJavaBlock::InverseCondition()
    {
        switch (Type) {
            case TYPE_CONDITION:
            case TYPE_CONDITION_TERNARY_OPERATOR:
            case TYPE_GOTO_IN_TERNARY_OPERATOR:
                MustInverseCondition ^= true;
                break;
            case TYPE_CONDITION_AND:
                Type = TYPE_CONDITION_OR;
                FirstSubBlockPtr->InverseCondition();
                SecondSubBlockPtr->InverseCondition();
                break;
            case TYPE_CONDITION_OR:
                Type = TYPE_CONDITION_AND;
                FirstSubBlockPtr->InverseCondition();
                SecondSubBlockPtr->InverseCondition();
                break;
            default:
                xel::Fatal("Invalid condition");
                break;
        }
    }

    xJavaSwitchCase::xJavaSwitchCase(xJavaBlock * BlockPtr)
    : DefaultCase(true), Offset(BlockPtr->FromOffset), BlockPtr(BlockPtr)
    {}

    xJavaSwitchCase::xJavaSwitchCase(size_t Value, xJavaBlock * BlockPtr)
    : DefaultCase(false), Value(Value), Offset(BlockPtr->FromOffset), BlockPtr(BlockPtr)
    {}

}
