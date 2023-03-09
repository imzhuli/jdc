#pragma once
#include "../base/_.hpp"
#include "../class_file/JD_Attribute.hpp"
#include <string>
#include <vector>

namespace jdc
{

    class xJavaCodeException
    {
    public:
        uint32_t Index;
        uint32_t StartPC;
        uint32_t EndPC;
        uint32_t HandlerPC;
        uint32_t CatchType;

    };

}