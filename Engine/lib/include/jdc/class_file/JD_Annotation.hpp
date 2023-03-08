#pragma once
#include "./JD_.hpp"
#include "../base/JD_ElementValue.hpp"
#include <string>
#include <vector>

namespace jdc
{

    struct xAnnotation
    {
        std::string                       Descriptor;
        std::vector<xElementValuePair>    ElementValuePairs;
    };

}
