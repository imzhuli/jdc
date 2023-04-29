#include <jdc/decompiler/JD_JavaBlock.hpp>

namespace jdc
{

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

}
