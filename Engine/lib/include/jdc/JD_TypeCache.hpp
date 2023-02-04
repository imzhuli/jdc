#pragma once
#include "./JD_Base.hpp"
#include "./JD_Class.hpp"
#include <vector>
#include <string>

namespace jdc
{

    struct xTypeInfo
    {
        std::string     TypeKey;
        eFieldType      Type;
        bool            Imported; // always false for native types
        std::string     FullTypeName;
        std::string     ShortTypeName;
        TypeKey         RelatedTypeKey;
    };



}
