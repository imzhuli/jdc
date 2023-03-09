#pragma once
#include "../base/_.hpp"
#include "../class_file/JD_ClassInfo.hpp"
#include <string>
#include <vector>

namespace jdc
{


    struct xJavaMethod
    {
        const xClassInfo *    ClassInfoPtr;
        const xMethodInfo *   MethodInfoPtr;
    };

}
