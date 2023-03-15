#pragma once
#include "../base/_.hpp"
#include "../class_file/JD_ClassInfo.hpp"
#include "./JD_JavaDeclaration.hpp"
#include <string>
#include <vector>

namespace jdc
{

    class xJavaClass;

    class xJavaMethod
    {
    public:
        const xJavaClass *    JavaClassPtr;
        const xMethodInfo *   MethodInfoPtr;
        std::string           OriginalName;
        std::string           Identifier;

        struct {
            xAttributeMap                 AttributeMap;
        } Extend;

        struct {
            xAnnotationDeclarations  AnnotationDeclarations;
        } Converted;

        X_PRIVATE_MEMBER void DoExtend();
        X_PRIVATE_MEMBER void DoConvert();
    };

}
