#pragma once
#include "./JD_.hpp"
#include <string>
#include <vector>

namespace jdc
{

    enum struct eBytecodeBehaviors : uint8_t
    {
        REF_GetField            = 1,
        REF_GetStatic           = 2,
        REF_PutField            = 3,
        REF_PutStatic           = 4,
        REF_InvokeVirtual       = 5,
        REF_InvokeStatic        = 6,
        REF_InvokeSpecial       = 7,
        REF_NewInvokeSpecial    = 8,
        REF_InvokeInterface     = 9,
    };

    using eReferenceKind = eBytecodeBehaviors;

}
