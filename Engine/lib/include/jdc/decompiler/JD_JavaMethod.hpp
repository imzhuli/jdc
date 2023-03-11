#pragma once
#include "../base/_.hpp"
#include "../class_file/JD_ClassInfo.hpp"
#include <string>
#include <vector>

namespace jdc
{


    class xJavaMethod
    {
    public:
        const xClassInfo *    ClassInfoPtr;
        const xMethodInfo *   MethodInfoPtr;
        std::string           OriginalName;
        std::string           Identifier;

        struct {
            xAttributeMap                 AttributeMap;



        } Extend;

        void DoExtend();
    };

}
