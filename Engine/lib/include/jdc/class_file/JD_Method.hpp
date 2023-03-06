#pragma once
#include "../base/JD_Base.hpp"
#include "./JD_Attribute.hpp"
#include <vector>

namespace jdc
{

    struct xMethodInfo
    {
        uint16_t                      AccessFlags;
        uint16_t                      NameIndex;
        uint16_t                      DescriptorIndex;
        std::vector<xAttributeInfo>   Attributes;
    };

}
