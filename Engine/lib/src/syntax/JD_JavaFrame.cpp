#include <jdc/syntax/JD_JavaFrame.hpp>

namespace jdc
{

    xJavaLocalVariable * xJavaLocalVariableSet::GetRoot(size_t Index)
    {
        if (Index >= Array.size()) {
            return nullptr;
        }
        auto VariablePtr = Array[Index];
        for(; VariablePtr; VariablePtr = VariablePtr->GetNext())
        {}
        return VariablePtr;
    }

    void xJavaLocalVariableSet::Add(size_t Index, xJavaLocalVariable * NewVariablePtr)
    {

        if (Index >= Array.size()) {
            Array.resize(Index * 2);
            Array[Index] = NewVariablePtr;
        }
        else {
            auto VariablePtr = Array[Index];

            if (!VariablePtr) {
                Array[Index] = NewVariablePtr;
            } else if (VariablePtr->GetFromOffset() < NewVariablePtr->GetFromOffset()) {
                assert(NewVariablePtr != VariablePtr);
                NewVariablePtr->SetNext(VariablePtr);
                Array[Index] = NewVariablePtr;
            } else {
                auto PreviousVariablePtr = VariablePtr;
                VariablePtr = VariablePtr->GetNext();

                while (VariablePtr && (VariablePtr->GetFromOffset() > NewVariablePtr->GetFromOffset())) {
                    PreviousVariablePtr = VariablePtr;
                    VariablePtr = VariablePtr->GetNext();
                }

                assert(PreviousVariablePtr != NewVariablePtr);
                PreviousVariablePtr->SetNext(NewVariablePtr);

                assert(NewVariablePtr != VariablePtr);
                NewVariablePtr->SetNext(VariablePtr);
            }
        }
        ++Size;
    }


    xJavaLocalVariable * xJavaLocalVariableSet::Remove(size_t Index, size_t Offset)
    {
        if (Index >= Array.size()) {
            return nullptr;
        }

        xJavaLocalVariable * PreviousVariablePtr = nullptr;
        xJavaLocalVariable * VariablePtr = Array[Index];

        while (VariablePtr) {
            if (VariablePtr->GetFromOffset() <= Offset) {
                if (!PreviousVariablePtr) {
                    Array[Index] = VariablePtr->GetNext();
                } else {
                    PreviousVariablePtr->SetNext(VariablePtr->GetNext());
                }

                --Size;
                VariablePtr->SetNext(nullptr);
                return VariablePtr;
            }

            PreviousVariablePtr = VariablePtr;
            assert(VariablePtr != VariablePtr->GetNext());
            VariablePtr = VariablePtr->GetNext();
        }
        return nullptr;
    }

    xJavaLocalVariable * xJavaLocalVariableSet::Get(size_t Index, size_t Offset)
    {
        if (Index >= Array.size()) {
            return nullptr;
        }

        auto VariablePtr = Array[Index];
        while (VariablePtr) {
            if (VariablePtr->GetFromOffset() <= Offset) {
                return VariablePtr;
            }

            assert(VariablePtr != VariablePtr->GetNext());
            VariablePtr = VariablePtr->GetNext();
        }

        return nullptr;
    }

    void xJavaLocalVariableSet::Update(size_t Index, size_t Offset, xJavaType * TypePtr)
    {
        if (Index >= Array.size()) {
            return;
        }

        auto VariablePtr = Array[Index];
        while (VariablePtr) {
            if (VariablePtr->GetFromOffset() == Offset) {
                VariablePtr->SetType(TypePtr);
                break;
            }
            assert(VariablePtr != VariablePtr->GetNext());
            VariablePtr = VariablePtr->GetNext();
        }
    }

}