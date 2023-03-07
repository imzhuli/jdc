#pragma once
#include "../base/JD_.hpp"
#include "./JD_Attribute.hpp"
#include <vector>

namespace jdc
{

    struct xFieldInfo
    {
        xAccessFlag                   AccessFlags;
        uint16_t                      NameIndex;
        uint16_t                      DescriptorIndex;
        std::vector<xAttributeInfo>   Attributes;
    };

}

