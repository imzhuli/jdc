#pragma once
#include "../base/JD_Base.hpp"
#include <xel/Byte.hpp>
#include <any>

namespace jdc
{

    struct xAttribute
    {
        virtual bool Parse(xel::xStreamReader & Reader, size_t RemainSize) = 0;
        virtual const char * GetName() const = 0;
        virtual const std::any GetValue() const = 0;
    };

}
