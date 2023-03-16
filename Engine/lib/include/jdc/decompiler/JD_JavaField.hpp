#pragma once
#include "../base/_.hpp"
#include "../class_file/JD_ClassInfo.hpp"
#include "./JD_JavaDeclaration.hpp"
#include <string>
#include <vector>

namespace jdc
{
    class xJavaClass;

    class xJavaField
    {
    public:
        const xJavaClass *           JavaClassPtr;
        const xFieldInfo *           FieldInfoPtr;
        std::string                  Name;
        std::string                  UnfixedTypeBinaryName;

        struct {
            std::string              FixedTypeCodeName;
            xAttributeMap            AttributeMap;
            xAnnotationDeclarations  AnnotationDeclarations;
        } Converted;

        X_PRIVATE_MEMBER void DoConvert();
    };

}
