#pragma once
#include "../base/_.hpp"
#include "../class_file/JD_Attribute.hpp"
#include <string>
#include <vector>

namespace jdc
{

    class xJavaBootstrapMethod
    {
    public:
        const xClassInfo *       ClassInfoPtr;
        uint16_t                 ReferenceIndex;
        std::vector<uint16_t>    ArgumentIndices;

    };


}
